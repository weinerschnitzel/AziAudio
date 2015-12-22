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
	int targetDMABuffer;
	int numFullBuffers = 0;

#ifdef _WIN32
	WaitForSingleObject(m_hMutex, INFINITE);
#else
	puts("[AI_LenChanged] To do:  Working non-Win32 mutex timing.");
#endif
	targetDMABuffer = m_AI_CurrentDMABuffer; // Target register
	if (m_AI_DMARemaining[targetDMABuffer] > 0)
	{
		targetDMABuffer = m_AI_CurrentDMABuffer ^ 1; // Set new target
		if (m_AI_DMARemaining[targetDMABuffer] > 0)
		{
			// FIFO is full.  We either need to ditch this buffer or wait
			if (configSyncAudio == true)
			{
#ifdef _WIN32
				ReleaseMutex(m_hMutex);
				while (m_AI_DMARemaining[targetDMABuffer] > 0)
					Sleep(1);
				WaitForSingleObject(m_hMutex, INFINITE);
#else
			//	To do:  Working non-Win32 mutex timing.
				while (m_AI_DMARemaining[targetDMABuffer] > 0)
					SDL_Delay(1);
#endif
				m_AI_DMABuffer[targetDMABuffer] = start;
				m_AI_DMARemaining[targetDMABuffer] = length;
			}
			numFullBuffers = 2;
			// TODO: How do we handle interrupts if we end up having to skip a buffer -- chances are if this occurs we 
			// are using FAT and an overflow occurred.  In that case we wouldn't need to handle the AI emulation in the
			// first place so we will skip it
		}
		else
		{
			// Fill last buffer
			m_AI_DMABuffer[targetDMABuffer] = start;
			m_AI_DMARemaining[targetDMABuffer] = length;
			numFullBuffers = 2;
		}
	}
	else
	{
		// Fill first buffer
		m_AI_DMABuffer[targetDMABuffer] = start;
		m_AI_DMARemaining[targetDMABuffer] = length;
		numFullBuffers = 1;
	}

	if (numFullBuffers > 0) *AudioInfo.AI_STATUS_REG |= AI_STATUS_DMA_BUSY;
	if (numFullBuffers > 1) *AudioInfo.AI_STATUS_REG |= AI_STATUS_FIFO_FULL;
#ifdef _WIN32
	ReleaseMutex(m_hMutex);
#endif
#endif
}

void SoundDriver::AI_SetFrequency(u32 Frequency)
{
#ifdef LEGACY_SOUND_DRIVER
	if (m_audioIsInitialized == true) SetFrequency(Frequency);
#else
	SetFrequency(Frequency);
	m_MaxBufferSize = ((Frequency * 4) / 10) & 0xFFFFFFF8; // 8 byte aligned
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
	retVal = m_AI_DMARemaining[m_AI_CurrentDMABuffer];
	ReleaseMutex(m_hMutex);
#else
	puts("[AI_ReadLength] To do:  working non-Win32 AI timing.");
	retVal = 0;
#endif
	return (retVal);
#endif
}

void SoundDriver::AI_Startup()
{
#ifdef LEGACY_SOUND_DRIVER	
	if (m_audioIsInitialized == true) DeInitialize();
	m_audioIsInitialized = (!Initialize() == TRUE);
	if (m_audioIsInitialized == true) SetVolume(configVolume);
#else
	Initialize();
	m_AI_CurrentDMABuffer = 0;
	m_AI_DMARemaining[0] = m_AI_DMARemaining[1] = 0;
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
	StartAudio();
#endif
}

void SoundDriver::AI_Shutdown()
{
#ifdef LEGACY_SOUND_DRIVER
	if (m_audioIsInitialized == true) DeInitialize();
	m_audioIsInitialized = false;
	//DeInitialize();
#else
	StopAudio();
	DeInitialize();
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
	StopAudio();
	if (m_audioIsInitialized) DeInitialize();
	m_audioIsInitialized = false;
	m_audioIsInitialized = (!Initialize() == TRUE);
	StartAudio();
}

void SoundDriver::AI_Update(Boolean Wait)
{
	AiUpdate(Wait);
}

// Copies data to the audio playback buffer
u32 SoundDriver::LoadAiBuffer(u8 *start, u32 length)
{
	u32 bytesToMove = length & 0xFFFFFFF8;
#ifndef LEGACY_SOUND_DRIVER
	u8 *ptrStart = start;
	u8 nullBuff[MAX_SIZE];
	if (start == NULL)
		ptrStart = nullBuff;

	assert((length & 0x3) == 0); // TODO: 4 or 8 byte alignment??

	m_DMAEnabled = (*AudioInfo.AI_CONTROL_REG & AI_CONTROL_DMA_ON) == AI_CONTROL_DMA_ON;

	if (bytesToMove > MAX_SIZE)
	{
		memset(ptrStart, 0, 100);
		return length;
	}

	if (m_DMAEnabled == false)
	{
		memset(ptrStart, 0, length);
		return length;
	}

	// Deplete stored buffer first
	while (bytesToMove > 0 && m_BufferRemaining > 0)
	{
		*(u32 *)(ptrStart) = *(u32 *)(m_Buffer + m_CurrentReadLoc);
		m_CurrentReadLoc += 4;
		ptrStart += 4;
		if (m_CurrentReadLoc > m_MaxBufferSize)
			m_CurrentReadLoc = 0;
		m_BufferRemaining -= 4;
		bytesToMove -= 4;
	}

#ifdef _WIN32
	WaitForSingleObject(m_hMutex, INFINITE);
#else
	puts("[LoadAIBuffer] To do:  non-Win32 m_hMutex");
#endif
	// Move any streamed audio samples
	while (bytesToMove > 0 && m_AI_DMARemaining[m_AI_CurrentDMABuffer] > 0)
	{
		*(u32 *)(ptrStart) = *(u32 *)(m_AI_DMABuffer[m_AI_CurrentDMABuffer]);
		m_AI_DMABuffer[m_AI_CurrentDMABuffer] += 4;
		ptrStart += 4;
		m_AI_DMARemaining[m_AI_CurrentDMABuffer] -= 4;
		bytesToMove -= 4;

		if (m_AI_DMARemaining[m_AI_CurrentDMABuffer] == 0)
		{
			m_AI_CurrentDMABuffer ^= 1; // Switch
			*AudioInfo.AI_STATUS_REG &= ~AI_STATUS_FIFO_FULL;
			if (m_AI_DMARemaining[m_AI_CurrentDMABuffer] == 0) *AudioInfo.AI_STATUS_REG &= ~AI_STATUS_DMA_BUSY;
			*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
			AudioInfo.CheckInterrupts();
		}
	}
	/*
	// Refill buffer
	while (m_BufferRemaining < m_MaxBufferSize && m_AI_DMARemaining[m_AI_CurrentDMABuffer] > 0)
	{
		*(u32 *)(m_Buffer + m_CurrentWriteLoc) = *(u32 *)(m_AI_DMABuffer[m_AI_CurrentDMABuffer]);
		m_CurrentWriteLoc += 4;
		m_AI_DMABuffer[m_AI_CurrentDMABuffer] += 4;
		m_AI_DMARemaining[m_AI_CurrentDMABuffer] -= 4;
		m_BufferRemaining += 4;

		if (m_AI_DMARemaining[m_AI_CurrentDMABuffer] == 0)
		{
			m_AI_CurrentDMABuffer ^= 1; // Switch
			*AudioInfo.AI_STATUS_REG &= ~AI_STATUS_FIFO_FULL;
			if (m_AI_DMARemaining[m_AI_CurrentDMABuffer] == 0) *AudioInfo.AI_STATUS_REG &= ~AI_STATUS_DMA_BUSY;
			*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
			AudioInfo.CheckInterrupts();
		}
	}
	*/
#ifdef _WIN32
	ReleaseMutex(m_hMutex);
#else
	// to do
#endif

	if (bytesToMove > 0)
	{
		memset(ptrStart, 0, bytesToMove);
		bytesToMove = 0;
	}
#else
	void *junk;
	junk = start;

#endif
	return (length - bytesToMove);
}
