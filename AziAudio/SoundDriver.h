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

#pragma once

#include <Windows.h>

#define SND_IS_NOT_EMPTY 0x4000000
#define SND_IS_FULL		 0x8000000

/*
 * Filters an integer for proximity within the range (base +/- leeway).
 * Created mostly just to solve the deviant N64 frequencies problem.
 */
extern long filter_range(long x, long base, int leeway);

class SoundDriver
{
public:
	bool configAIEmulation;
	bool configSyncAudio;
	bool configForceSync;
	bool configMute;
	bool configHLE;
	bool configRSP;
	unsigned long configVolume;
	char configAudioLogFolder[500];
	char configDevice[100];

	// Setup and Teardown Functions
	virtual BOOL Initialize(HWND hwnd) = 0;
	virtual void DeInitialize() = 0;

	// Buffer Functions for the Audio Code
	virtual void SetFrequency(DWORD Frequency) = 0;		// Sets the Nintendo64 Game Audio Frequency
	virtual DWORD AddBuffer(BYTE *start, DWORD length) = 0;	// Uploads a new buffer and returns status
	//virtual void FillBuffer(BYTE *buff, DWORD len) = 0;
	//virtual void SetSegmentSize(DWORD length) = 0;

	// Management functions
	virtual void AiUpdate(BOOL Wait) = 0;
	virtual void StopAudio() = 0;							// Stops the Audio PlayBack (as if paused)
	virtual void StartAudio() = 0;							// Starts the Audio PlayBack (as if unpaused)

	virtual DWORD GetReadStatus() = 0;						// Returns the status on the read pointer

	virtual void SetVolume(DWORD volume) = 0;
	virtual ~SoundDriver() {};

protected:
	SoundDriver(){
		configAIEmulation = true;
		configSyncAudio = true;
		configForceSync = false;
		configMute = false;
		configHLE = true;
		configRSP = true;
		configVolume = 0;
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
		strcpy_s(configAudioLogFolder, 500, "D:\\");
		strcpy_s(configDevice, 100, "");
#else
		strcpy(configAudioLogFolder, "D:\\");
		strcpy(configDevice, "");
#endif
	}
};
