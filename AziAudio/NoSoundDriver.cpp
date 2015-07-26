/*
	NoSound Driver to demonstrate how to use the SoundDriver interface
*/

#include "NoSoundDriver.h"

BOOL NoSoundDriver::Initialize()
{
	BOOL result;
	result = QueryPerformanceFrequency(&perfFreq);
	result = QueryPerformanceCounter(&perfLast) || result;
	dllInitialized = true;
	isPlaying = false;
	return result;
}

void NoSoundDriver::DeInitialize()
{
	isPlaying = false;
	dllInitialized = false;
}


// Management functions
void NoSoundDriver::AiUpdate(BOOL Wait)
{
	LARGE_INTEGER sampleInterval;
	long samples;
	if (Wait == TRUE) Sleep(1);
	if (isPlaying == true && countsPerSample.QuadPart > 0)
	{
		QueryPerformanceCounter(&perfTimer);
		sampleInterval.QuadPart = perfTimer.QuadPart - perfLast.QuadPart;
		samples = (long)(sampleInterval.QuadPart / countsPerSample.QuadPart);
		if (samples > 0)
		{
			perfLast.QuadPart = perfTimer.QuadPart;// += countsPerSample.QuadPart * samples;
			LoadAiBuffer(NULL, samples * 4); // NULL means it won't actually try to fill a buffer
		}
	}
}

void NoSoundDriver::StopAudio()
{
	isPlaying = false;
}

void NoSoundDriver::StartAudio()
{
	QueryPerformanceCounter(&perfLast);
	isPlaying = true;
}

void NoSoundDriver::SetFrequency(DWORD Frequency)
{
	int SamplesPerSecond = Frequency; // 16 bit * stereo

	// Must determine the number of Counter units per Sample
	QueryPerformanceFrequency(&perfFreq); // Counters per Second

	countsPerSample.QuadPart = perfFreq.QuadPart / SamplesPerSecond;
	QueryPerformanceCounter(&perfTimer);
	perfLast.QuadPart = perfTimer.QuadPart;
}
