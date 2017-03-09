#pragma once

//#include "common.h"
#ifdef _WIN32
#include <Windows.h>
#endif

class Configuration
{
protected:
	static const int MAX_FOLDER_LENGTH = 500;
	static const int MAX_DEVICE_LENGTH = 100;
	static INT_PTR CALLBACK ConfigProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
	static void LoadSettings();
	static void SaveSettings();
	static bool configAIEmulation;
	static bool configSyncAudio;
	static bool configForceSync;
	static unsigned long configVolume;
	static char configAudioLogFolder[MAX_FOLDER_LENGTH];
	static char configDevice[MAX_DEVICE_LENGTH];
	static unsigned long configFrequency;
	static unsigned long configBitRate;
	static unsigned long configBufferLevel; // 1-9
	static unsigned long configBufferFPS;
	static unsigned long configBackendFPS;

public:
	static void LoadDefaults();
#ifdef _WIN32
	static void ConfigDialog(HWND hParent);
	static void AboutDialog(HWND hParent);
#endif
	// Accessors for the Configuration variables to prevent changes outside of Configuration.cpp
	static bool getAIEmulation() { return configAIEmulation; }
	static bool getSyncAudio()   { return configSyncAudio; }
	static bool getForceSync()  { return configForceSync; }
	static unsigned long getVolume() { return configVolume; }
	static char* getAudioLogFolder() {
		static char retVal[MAX_FOLDER_LENGTH];
		strcpy(retVal, configAudioLogFolder);
		return retVal;
	}
	static char* getDevice() {
		static char retVal[MAX_DEVICE_LENGTH];
		strcpy(retVal, configDevice);
		return retVal;
	}
	static unsigned long getFrequency() { return configFrequency; }
	static unsigned long getBitRate() { return configBitRate; }
	static unsigned long getBufferLevel() { return configBufferLevel; }
	static unsigned long getBufferFPS() { return configBufferFPS; }
	static unsigned long getBackendFPS() { return configBackendFPS; }

};
