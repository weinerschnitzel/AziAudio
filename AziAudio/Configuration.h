#pragma once

//#include "common.h"


class Configuration
{
private:

public:
	// Configuration variables
	static bool configAIEmulation;
	static bool configSyncAudio;
	static bool configForceSync;
	static unsigned long configVolume;
	static char configAudioLogFolder[500];
	static char configDevice[100];
};