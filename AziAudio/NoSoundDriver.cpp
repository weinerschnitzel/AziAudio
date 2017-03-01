/*
	NoSound Driver to demonstrate how to use the SoundDriver interface
*/
#include "NoSoundDriver.h"

Boolean NoSoundDriver::Initialize()
{
	/*
	Boolean result;
#ifdef _WIN32
	result = QueryPerformanceFrequency(&perfFreq);
	result = QueryPerformanceCounter(&perfLast) || result;
#else
	result = false;
#endif
	*/
	dllInitialized = true;
	isPlaying = false;
	m_SamplesPerSecond = false;
	return true;
}

void NoSoundDriver::DeInitialize()
{
	isPlaying = false;
	dllInitialized = false;
}

// Management functions
void NoSoundDriver::AiUpdate(Boolean Wait)
{
	//LARGE_INTEGER sampleInterval;
	long samples;

	//if (Wait)
	//	WaitMessage(); 
#if defined(_WIN32) || defined(_XBOX)
	if (Wait == TRUE)
		Sleep(1);
#else
	if (Wait == TRUE)
		SDL_Delay(10);
#endif
	
	if (isPlaying == true)
	{
		samples = m_SamplesPerSecond / (1000 / 1);
		LoadAiBuffer(NULL, samples * 4);
	}
	
	/*
	if (isPlaying == true && countsPerSample.QuadPart > 0)
	{
#ifdef _WIN32
		QueryPerformanceCounter(&perfTimer);
#else
		// To do:  Replace this with SDL, or Linux audio won't play.
#endif
		sampleInterval.QuadPart = perfTimer.QuadPart - perfLast.QuadPart;
		samples = (long)(sampleInterval.QuadPart / countsPerSample.QuadPart);
		if (samples > 0)
		{
			perfLast.QuadPart = perfTimer.QuadPart;// += countsPerSample.QuadPart * samples;
			LoadAiBuffer(NULL, samples * 4); // NULL means it won't actually try to fill a buffer
		}
	}
	*/
}

void NoSoundDriver::StopAudio()
{
	isPlaying = false;
}

void NoSoundDriver::StartAudio()
{
	/*
#ifdef _WIN32
	QueryPerformanceCounter(&perfLast);
#endif
	*/
	isPlaying = true;
}

void NoSoundDriver::SetFrequency(u32 Frequency)
{
#ifdef _WIN32
	//int SamplesPerSecond = Frequency; // 16 bit * stereo

	// Must determine the number of Counter units per Sample
	/*QueryPerformanceFrequency(&perfFreq); // Counters per Second

	countsPerSample.QuadPart = perfFreq.QuadPart / SamplesPerSecond;
	QueryPerformanceCounter(&perfTimer);
	perfLast.QuadPart = perfTimer.QuadPart;*/
#else
	// To do:  Replace this with SDL, or Linux audio won't play.
#endif
}
