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

#include "common.h"
#ifdef USE_XAUDIO2
#include "XAudio2SoundDriver.h"
#include "AudioSpec.h"
#include <stdio.h>

static IXAudio2* g_engine;
static IXAudio2SourceVoice* g_source;
static IXAudio2MasteringVoice* g_master;

static bool audioIsPlaying = false;
static bool canPlay = false;

static BYTE bufferData[10][44100 * 4];
static int bufferLength[10];
static int writeBuffer = 0;
static int readBuffer = 0;
static int filledBuffers;
static int bufferBytes;
static int lastLength = 1;

static int cacheSize = 0;
static int interrupts = 0;
static VoiceCallback voiceCallback;
XAudio2SoundDriver::XAudio2SoundDriver()
{
	g_engine = NULL;
	g_source = NULL;
	g_master = NULL;
	dllInitialized = false;
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
}


XAudio2SoundDriver::~XAudio2SoundDriver()
{
	DeInitialize();
	//Teardown();
	CoUninitialize();
}
static HANDLE hMutex;


BOOL XAudio2SoundDriver::Initialize()
{
	if (g_source != NULL)
	{
		g_source->Start();
	}
	bufferLength[0] = bufferLength[1] = bufferLength[2] = bufferLength[3] = bufferLength[4] = bufferLength[5] = 0;
	bufferLength[6] = bufferLength[7] = bufferLength[8] = bufferLength[9] = 0;
	audioIsPlaying = false;
	writeBuffer = 0;
	readBuffer = 0;
	filledBuffers = 0;
	bufferBytes = 0;
	lastLength = 1;

	cacheSize = 0;
	interrupts = 0;
	return false;
}

BOOL XAudio2SoundDriver::Setup()
{
	if (dllInitialized == true) return true;
	dllInitialized = true;
	bufferLength[0] = bufferLength[1] = bufferLength[2] = bufferLength[3] = bufferLength[4] = bufferLength[5] = 0;
	bufferLength[6] = bufferLength[7] = bufferLength[8] = bufferLength[9] = 0;
	audioIsPlaying = false;
	writeBuffer = 0;
	readBuffer = 0;
	filledBuffers = 0;
	bufferBytes = 0;
	lastLength = 1;

	cacheSize = 0;
	interrupts = 0;

	hMutex = CreateMutex(NULL, FALSE, NULL);
	if (FAILED(XAudio2Create(&g_engine)))
	{
		CoUninitialize();
		return -1;
	}

	if (FAILED(g_engine->CreateMasteringVoice(&g_master)))
	{
		g_engine->Release();
		CoUninitialize();
		return -2;
	}
	canPlay = true;

	// Load Wave File

	WAVEFORMATEX wfm;

	memset(&wfm, 0, sizeof(WAVEFORMATEX));

	wfm.wFormatTag = WAVE_FORMAT_PCM;
	wfm.nChannels = 2;
	wfm.nSamplesPerSec = 44100;
	wfm.wBitsPerSample = 16; // TODO: Allow 8bit audio...
	wfm.nBlockAlign = wfm.wBitsPerSample / 8 * wfm.nChannels;
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;


	if (FAILED(g_engine->CreateSourceVoice(&g_source, &wfm, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &voiceCallback, NULL, NULL)))
	{
		g_engine->Release();
		CoUninitialize();
		return -3;
	}

	g_source->Start();
	
	return FALSE;
}
void XAudio2SoundDriver::DeInitialize()
{
	Teardown();
	/*
	if (g_source != NULL)
	{
		g_source->Stop();
		g_source->FlushSourceBuffers();
		//g_source->DestroyVoice();
	}*/

}

void XAudio2SoundDriver::Teardown()
{
	if (dllInitialized == false) return;
	if (hMutex != NULL)
		WaitForSingleObject(hMutex, INFINITE);
	if (g_source != NULL)
	{
		g_source->Stop();
		g_source->FlushSourceBuffers();
		g_source->DestroyVoice();
	}
	
	if (g_master != NULL) g_master->DestroyVoice();
	if (g_engine != NULL && canPlay)
	{
		g_engine->StopEngine();
		g_engine->Release();
	}
	g_engine = NULL;
	g_master = NULL;
	g_source = NULL;
	if (hMutex != NULL)
	{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}
	hMutex = NULL;
	dllInitialized = false;
}

void XAudio2SoundDriver::SetFrequency(DWORD Frequency)
{
	cacheSize = (Frequency / 25) * 4;// (((Frequency * 4) / 100) & ~0x3) * 8;
	if (Setup() < 0) /* failed to apply a sound device */
		return;
	g_source->SetSourceSampleRate(Frequency);
}

DWORD XAudio2SoundDriver::AddBuffer(BYTE *start, DWORD length)
{
	if (length == 0 || g_source == NULL) {
		*AudioInfo.AI_STATUS_REG = 0;
		*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
		AudioInfo.CheckInterrupts();
		return 0;
	}
	lastLength = length;

	// Gracefully waiting for filled buffers to deplete
	if (configSyncAudio == true || configForceSync == true)
		while (filledBuffers == 10) Sleep(1);
	else 
		if (filledBuffers == 10) return 0;
		
	WaitForSingleObject(hMutex, INFINITE);
	for (DWORD x = 0; x < length; x += 4)
	{
		bufferData[writeBuffer][x] = start[x + 2];
		bufferData[writeBuffer][x+1] = start[x + 3];
		bufferData[writeBuffer][x+2] = start[x];
		bufferData[writeBuffer][x+3] = start[x + 1];
	}
	bufferLength[writeBuffer] = length;
	bufferBytes += length;
	filledBuffers++;

	XAUDIO2_BUFFER xa2buff;

	xa2buff.Flags = XAUDIO2_END_OF_STREAM; // Suppress XAudio2 warnings
	xa2buff.PlayBegin = 0;
	xa2buff.PlayLength = 0;
	xa2buff.LoopBegin = 0;
	xa2buff.LoopLength = 0;
	xa2buff.LoopCount = 0;
	xa2buff.pContext = &bufferLength[writeBuffer];
	xa2buff.AudioBytes = length;
	xa2buff.pAudioData = bufferData[writeBuffer];
	if (canPlay)
		g_source->SubmitSourceBuffer(&xa2buff);

	++writeBuffer;
	writeBuffer %= 10;

	if (bufferBytes < cacheSize || configForceSync == true)
	{
		*AudioInfo.AI_STATUS_REG = AI_STATUS_DMA_BUSY;
		*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
		AudioInfo.CheckInterrupts();
	}
	else
	{
		if (filledBuffers >= 2)
			*AudioInfo.AI_STATUS_REG |= AI_STATUS_FIFO_FULL;
		interrupts++;
	}
	ReleaseMutex(hMutex);
	
	return 0;
}

void XAudio2SoundDriver::AiUpdate(BOOL Wait)
{
	if (Wait)
		WaitMessage();
}

void XAudio2SoundDriver::StopAudio()
{
	
	audioIsPlaying = false;
	if (g_source != NULL)
		g_source->FlushSourceBuffers();
		
}

void XAudio2SoundDriver::StartAudio()
{
	audioIsPlaying = true;
}

DWORD XAudio2SoundDriver::GetReadStatus()
{
	XAUDIO2_VOICE_STATE xvs;
	int retVal;

	if (canPlay)
		g_source->GetState(&xvs);
	else
		return 0;

//	printf("%i - %i - %i\n", xvs.SamplesPlayed, bufferLength[0], bufferLength[1]);

	if (xvs.BuffersQueued == 0 || configForceSync == true) return 0;

	if (bufferBytes + lastLength < cacheSize)
		return 0;
	else
		retVal = (lastLength - xvs.SamplesPlayed * 4) & ~0x7;// bufferBytes % lastLength;// *(int *)xvs.pCurrentBufferContext - (int)xvs.SamplesPlayed;

	if (retVal < 0) return 0; else return retVal % lastLength;
//	return 0;
}

// 100 - Mute to 0 - Full Volume
void XAudio2SoundDriver::SetVolume(DWORD volume)
{
	float xaVolume = 1.0f - ((float)volume / 100.0f);
	if (g_source != NULL) g_source->SetVolume(xaVolume);
	//XAUDIO2_MAX_VOLUME_LEVEL
}


void __stdcall VoiceCallback::OnBufferEnd(void * pBufferContext)
{
	WaitForSingleObject(hMutex, INFINITE);
#ifdef SEH_SUPPORTED
	__try // PJ64 likes to close objects before it shuts down the DLLs completely...
	{
#endif
		*AudioInfo.AI_STATUS_REG = AI_STATUS_DMA_BUSY;
		if (interrupts > 0)
		{
			interrupts--;
			*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
			AudioInfo.CheckInterrupts();
		}
#ifdef SEH_SUPPORTED
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
#endif
	bufferBytes -= *(int *)(pBufferContext);
	filledBuffers--;
	ReleaseMutex(hMutex);
}

void __stdcall VoiceCallback::OnVoiceProcessingPassStart(UINT32 SamplesRequired) 
{
	UNREFERENCED_PARAMETER(SamplesRequired);
	//if (SamplesRequired > 0)
	//	printf("SR: %i FB: %i BB: %i  CS:%i\n", SamplesRequired, filledBuffers, bufferBytes, cacheSize);
}
#endif
