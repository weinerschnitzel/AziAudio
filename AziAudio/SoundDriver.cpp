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
void SoundDriver::AI_LenChanged(BYTE *start, DWORD length)
{
	if (m_audioIsInitialized == false)
	{
		*AudioInfo.AI_STATUS_REG = AI_STATUS_DMA_BUSY;
		*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
	}
	else
	{
		AddBuffer(start, length);
	}
}

void SoundDriver::AI_SetFrequency(DWORD Frequency)
{
	if (m_audioIsInitialized == true) SetFrequency(Frequency);
}

DWORD SoundDriver::AI_ReadLength()
{
	if (m_audioIsInitialized == false) return 0;
	return GetReadStatus();
}

void SoundDriver::AI_Startup(HWND hwnd)
{
	m_hwnd = hwnd;
	if (m_audioIsInitialized == true) DeInitialize();
	m_audioIsInitialized = (!Initialize(m_hwnd) == TRUE);
	if (m_audioIsInitialized == true) SetVolume(configVolume);
}

void SoundDriver::AI_Shutdown()
{
	if (m_audioIsInitialized == true) DeInitialize();
	m_audioIsInitialized = false;
	//DeInitialize();
}

void SoundDriver::AI_ResetAudio()
{
	if (m_audioIsInitialized) DeInitialize();
	m_audioIsInitialized = false;
	m_audioIsInitialized = (!Initialize(m_hwnd) == TRUE);
}