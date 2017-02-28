/****************************************************************************
*                                                                           *
* Azimer's HLE Audio Plugin for Project64 Compatible N64 Emulators          *
* http://www.apollo64.com/                                                  *
* Copyright (C) 2000-2015 Azimer. All rights reserved.                      *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/

#include "SoundDriver.h"

// Load the buffer from the AI interface to our emulated buffer
void SoundDriver::AI_LenChanged(u8 *start, u32 length)
{
#ifdef LEGACY_SOUND_DRIVER
	if (m_audioIsInitialized == false)
	{
		*AudioInfo.AI_STATUS_REG = AI_STATUS_DMA_BUSY;
		*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
	}
	else
	{
		AddBuffer(start, length);
	}
#else
	// Bleed off some of this buffer to smooth out audio
	if (length < m_MaxBufferSize && configSyncAudio == true)
	{
		while ((m_BufferRemaining) == m_MaxBufferSize)// ((m_MaxBufferSize / 3) * 2))
		{
#ifdef _WIN32
			Sleep(1);
#else
			// TODO: We need to support non-windows in another way
			assert(0);
#endif
		}
	}

#ifdef _WIN32
	WaitForSingleObject(m_hMutex, INFINITE);
#else
	puts("[AI_LenChanged] To do:  Working non-Win32 mutex timing.");
#endif
	BufferAudio();
	if (m_AI_DMASecondaryBytes > 0)
	{
		// FIFO is full.  We either need to ditch this buffer or wait
		dprintf("X");
		// TODO: How do we handle interrupts if we end up having to skip a buffer -- chances are if this occurs we 
		// are using FAT and an overflow occurred.  In that case we wouldn't need to handle the AI emulation in the
		// first place so we will skip it
	}

	m_AI_DMASecondaryBuffer = start;
	m_AI_DMASecondaryBytes = length;
	if (m_AI_DMAPrimaryBytes == 0)
	{
		m_AI_DMAPrimaryBuffer = m_AI_DMASecondaryBuffer; m_AI_DMASecondaryBuffer = NULL;
		m_AI_DMAPrimaryBytes = m_AI_DMASecondaryBytes; m_AI_DMASecondaryBytes = 0; 
	}

	if (configAIEmulation == true)
	{
		*AudioInfo.AI_STATUS_REG = AI_STATUS_DMA_BUSY;
		if (m_AI_DMAPrimaryBytes > 0 && m_AI_DMASecondaryBytes > 0)
		{
			*AudioInfo.AI_STATUS_REG = AI_STATUS_DMA_BUSY | AI_STATUS_FIFO_FULL;
		}
	}
	BufferAudio();

#ifdef _WIN32
	ReleaseMutex(m_hMutex);
#endif
#endif
}

void SoundDriver::AI_SetFrequency(u32 Frequency)
{
	m_SamplesPerSecond = Frequency;
#ifdef LEGACY_SOUND_DRIVER
	if (m_audioIsInitialized == true) SetFrequency(Frequency);
#else
	SetFrequency(Frequency);
	m_MaxBufferSize = (u32)((Frequency/90)) * 4 * 2; // TODO: Make this configurable
	m_BufferRemaining = 0;
	m_CurrentReadLoc = m_CurrentWriteLoc = m_BufferRemaining = 0;
#endif
}

u32 SoundDriver::AI_ReadLength()
{
#ifdef LEGACY_SOUND_DRIVER
	if (m_audioIsInitialized == false) return 0;
	return GetReadStatus();
#else
	u32 retVal;

#ifdef _WIN32
	WaitForSingleObject(m_hMutex, INFINITE);
#endif
	retVal = m_AI_DMAPrimaryBytes;
#ifdef _WIN32
	ReleaseMutex(m_hMutex);
#endif
	return (retVal & ~0x3);
#endif
}

void SoundDriver::AI_Startup()
{
#ifdef LEGACY_SOUND_DRIVER	
	if (m_audioIsInitialized == true) DeInitialize();
	m_audioIsInitialized = false;
	m_audioIsInitialized = (Initialize() == FALSE);
	if (m_audioIsInitialized == true) SetVolume(configVolume);
#else
	Initialize();
	m_AI_DMAPrimaryBytes = m_AI_DMASecondaryBytes = 0;
	m_AI_DMAPrimaryBuffer = m_AI_DMASecondaryBuffer = NULL;

	m_MaxBufferSize = MAX_SIZE;
	m_CurrentReadLoc = m_CurrentWriteLoc = m_BufferRemaining = 0;
	m_DMAEnabled = false;

#ifdef _WIN32
	if (m_hMutex == NULL)
	{
		m_hMutex = CreateMutex(NULL, FALSE, NULL);
	}
#else
	// to do
#endif
#endif
	StartAudio();
}

void SoundDriver::AI_Shutdown()
{
	StopAudio();
#ifdef LEGACY_SOUND_DRIVER
	if (m_audioIsInitialized == true) DeInitialize();
	m_audioIsInitialized = false;
	//DeInitialize();
#else
	DeInitialize();
	m_BufferRemaining = 0;
#ifdef _WIN32
	if (m_hMutex != NULL)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
#else
	// to do
#endif
#endif
}

void SoundDriver::AI_ResetAudio()
{
#ifndef LEGACY_SOUND_DRIVER
	m_BufferRemaining = 0;
#endif
	if (m_audioIsInitialized == true) AI_Shutdown();
	DeInitialize();
	m_audioIsInitialized = false;
	AI_Startup();
	m_audioIsInitialized = (Initialize() == FALSE);
	StartAudio();
}

void SoundDriver::AI_Update(Boolean Wait)
{
	AiUpdate(Wait);
}

#ifndef LEGACY_SOUND_DRIVER
void SoundDriver::BufferAudio()
{
	while ((m_BufferRemaining < m_MaxBufferSize) && (m_AI_DMAPrimaryBytes > 0 || m_AI_DMASecondaryBytes > 0))
	{
		*(u16 *)(m_Buffer + m_CurrentWriteLoc) = *(u16 *)(m_AI_DMAPrimaryBuffer+2);
		*(u16 *)(m_Buffer + m_CurrentWriteLoc+2) = *(u16 *)m_AI_DMAPrimaryBuffer;
		m_CurrentWriteLoc += 4;
		m_AI_DMAPrimaryBuffer += 4;
		m_CurrentWriteLoc %= m_MaxBufferSize;
		assert(m_BufferRemaining <= m_MaxBufferSize);
		m_BufferRemaining += 4;
		assert(m_BufferRemaining <= m_MaxBufferSize);
		m_AI_DMAPrimaryBytes -= 4;
		if (m_AI_DMAPrimaryBytes == 0)
		{
			m_AI_DMAPrimaryBytes = m_AI_DMASecondaryBytes; m_AI_DMAPrimaryBuffer = m_AI_DMASecondaryBuffer; // Switch
			m_AI_DMASecondaryBytes = 0; m_AI_DMASecondaryBuffer = NULL;
			if (configAIEmulation == true)
			{
				*AudioInfo.AI_STATUS_REG = AI_STATUS_DMA_BUSY;
				*AudioInfo.AI_STATUS_REG &= ~AI_STATUS_FIFO_FULL;
				*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
				AudioInfo.CheckInterrupts();
				if (m_AI_DMAPrimaryBytes == 0) *AudioInfo.AI_STATUS_REG = 0;
			}
		}
	}
}

// Copies data to the audio playback buffer
u32 SoundDriver::LoadAiBuffer(u8 *start, u32 length)
{
	u32 bytesToMove = length;// &0xFFFFFFFC;
	u8 *ptrStart = start;
	u8 nullBuff[MAX_SIZE];
	u32 writePtr = 0;
	if (start == NULL)
		ptrStart = nullBuff;

	assert((length & 0x3) == 0);
	assert(bytesToMove <= m_MaxBufferSize);  // We shouldn't be asking for more.

	m_DMAEnabled = (*AudioInfo.AI_CONTROL_REG & AI_CONTROL_DMA_ON) == AI_CONTROL_DMA_ON;

	if (bytesToMove > m_MaxBufferSize)
	{
		memset(ptrStart, 0, 100);
		return length;
	}

	// Return silence -- DMA is disabled
	if (m_DMAEnabled == false)
	{
		memset(ptrStart, 0, length);
		return length;
	}

#ifdef _WIN32
	WaitForSingleObject(m_hMutex, INFINITE);
#else
	puts("[LoadAIBuffer] To do:  non-Win32 m_hMutex");
#endif

	// Step 0: Replace depleted stored buffer for next run
	BufferAudio();

	// Step 1: Deplete stored buffer (should equal length size)
	if (bytesToMove <= m_BufferRemaining)
	{
		while (bytesToMove > 0 && m_BufferRemaining > 0)
		{
			*(u32 *)(ptrStart+writePtr) = *(u32 *)(m_Buffer + m_CurrentReadLoc);
			m_CurrentReadLoc += 4;
			writePtr += 4;
			m_CurrentReadLoc %= m_MaxBufferSize;
			assert(m_BufferRemaining <= m_MaxBufferSize);
			m_BufferRemaining -= 4;
			assert(m_BufferRemaining <= m_MaxBufferSize);
			bytesToMove -= 4;
		}
		assert(writePtr == length);
	}

	// Step 2: Fill bytesToMove with silence
	if (bytesToMove == length)
		dprintf("S");

	while (bytesToMove > 0)
	{
		*(u8 *)(ptrStart + writePtr) = 0;
		writePtr += 1;
		bytesToMove -= 1;
	}

	// Step 3: Replace depleted stored buffer for next run
	BufferAudio();

#ifdef _WIN32
	ReleaseMutex(m_hMutex);
#else
	// to do
#endif
	assert(bytesToMove == 0);
	return (length - bytesToMove);
}
#endif
