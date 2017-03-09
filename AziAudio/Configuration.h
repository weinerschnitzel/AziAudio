#pragma once

//#include "common.h"
#ifdef _WIN32
#include <Windows.h>
#endif

class Configuration
{
private:

public:
	static void LoadDefaults();
#ifdef _WIN32
	static void ConfigDialog(HWND hParent);
	static void AboutDialog(HWND hParent);
#endif
	// Configuration variables
	static bool configAIEmulation;
	static bool configSyncAudio;
	static bool configForceSync;
	static unsigned long configVolume;
	static char configAudioLogFolder[500];
	static char configDevice[100];
	static unsigned long configFrequency;
	static unsigned long configBufferLevel; // 1-9
};