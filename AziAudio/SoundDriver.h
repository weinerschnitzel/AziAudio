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

#include "common.h"
#include "AudioSpec.h"

#define SND_IS_NOT_EMPTY 0x4000000
#define SND_IS_FULL		 0x8000000

class SoundDriver
{
public:
	// Configuration variables
	// TODO: these may need to go elsewhere
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

	// Management functions
	virtual void AiUpdate(BOOL Wait) = 0;
	virtual void StopAudio() = 0;							// Stops the Audio PlayBack (as if paused)
	virtual void StartAudio() = 0;							// Starts the Audio PlayBack (as if unpaused)
	virtual void SetFrequency(DWORD Frequency) = 0;		// Sets the Nintendo64 Game Audio Frequency

	// Deprecated
	virtual DWORD GetReadStatus() = 0;						// Returns the status on the read pointer
	virtual DWORD AddBuffer(BYTE *start, DWORD length) = 0;	// Uploads a new buffer and returns status

	// Audio Spec interface methods (new)
	void AI_SetFrequency(DWORD Frequency);
	void AI_LenChanged(BYTE *start, DWORD length);
	DWORD AI_ReadLength();
	void AI_Startup(HWND hwnd);
	void AI_Shutdown();
	void AI_ResetAudio();

	// Buffer Management methods
	DWORD LoadAiBuffer(BYTE *start, DWORD length);


	virtual void SetVolume(DWORD volume) = 0;
	virtual ~SoundDriver() {};

protected:
	// Temporary (to allow for incremental development)
	bool m_audioIsInitialized;
	HWND m_hwnd;

	// Variables for AI DMA emulation
	int m_AI_CurrentDMABuffer; // Currently playing AI Buffer
	BYTE *m_AI_DMABuffer[2];    // Location in RDRAM containing buffer data
	DWORD m_AI_DMARemaining[2]; // How much RDRAM buffer is left to read

	static const int MAX_SIZE = 44100 * 2 * 2; // Max Buffer Size (44100Hz * 16bit * Stereo)
	static const int NUM_BUFFERS = 4; // Number of emulated buffers
	int m_MaxBufferSize;   // Variable size determined by Playback rate
	int m_CurrentReadBuffer;   // Currently playing Buffer
	int m_CurrentWriteBuffer;  // Currently writing Buffer
	u8 m_Buffer[NUM_BUFFERS][MAX_SIZE]; // Emulated buffers
	u32 m_BufferRemaining[NUM_BUFFERS]; // Buffer remaining
	bool m_DMAEnabled;  // Sets to true when DMA is enabled

	SoundDriver(){
		m_audioIsInitialized = false;
		configAIEmulation = true;
		configSyncAudio = true;
		configForceSync = false;
		configMute = false;
		configHLE = true;
		configRSP = true;
		configVolume = 0;
#if (_MSC_VER > 1400) && !defined(_CRT_SECURE_NO_WARNINGS)
		strcpy_s(configAudioLogFolder, 500, "D:\\");
		strcpy_s(configDevice, 100, "");
#else
		strcpy(configAudioLogFolder, "D:\\");
		strcpy(configDevice, "");
#endif
	}
};
