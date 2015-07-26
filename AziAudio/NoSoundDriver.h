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
/*
NoSound Driver to demonstrate how to use the SoundDriver interface
*/

#pragma once
#include "common.h"
#include "SoundDriver.h"


class NoSoundDriver :
	public SoundDriver
{
public:
	NoSoundDriver() {};
	~NoSoundDriver() {};

	// Setup and Teardown Functions
	BOOL Initialize();
	void DeInitialize();

	// Management functions
	void AiUpdate(BOOL Wait);
	void StopAudio();
	void StartAudio();
	void SetFrequency(DWORD Frequency);

protected:

	bool dllInitialized;
	LARGE_INTEGER perfTimer;
	LARGE_INTEGER perfFreq;
	LARGE_INTEGER perfLast;
	LARGE_INTEGER countsPerSample;

	bool isPlaying;
};