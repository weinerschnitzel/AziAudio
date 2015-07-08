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

#ifndef _COMMON_DOT_H_
#define _COMMON_DOT_H_

#if defined (_XBOX)
#include <xtl.h>
#include "XBox/xbox_depp.h"
#else
#include <windows.h>
#include <commctrl.h>
#endif

#include <stdio.h>
#include <assert.h>

//#define ENABLEPROFILING

#if defined(_MSC_VER)
#define SEH_SUPPORTED
#endif

#ifndef XAUDIO_LIBRARIES_UNAVAILABLE
#define USE_XAUDIO2
#endif

#include "my_types.h"

typedef struct {
	u16 Version;
	u32 BufferSize;
	BOOL doAIHACK;
	BOOL syncAudio;
	BOOL fillAudio;
	BOOL oldStyle;
	BOOL Reserved2;
	BOOL Reserved3;
	u32  Reserved4;
	u32  Reserved5;
	u32  Reserved6;
} rSettings;
extern rSettings RegSettings;
#endif

#define AUDIOCODE 0
#define HLECODE   1
#define CPUCODE   2

unsigned long GenerateCRC (unsigned char *data, int size);

#ifdef USE_XAUDIO2
#define PLUGIN_NAME     "XA2 Audio"
#else
#define PLUGIN_NAME     "DS8 Audio"
#endif

#ifdef DEVBUILD
#ifdef __GNUC__
#define PLUGIN_BUILDSYS "Mingw"
#else
#define PLUGIN_BUILDSYS "MSVC"
#endif
#ifdef _DEBUG
#define PLUGIN_DEBUG " (" PLUGIN_BUILDSYS " Debug)"
#else
#define PLUGIN_DEBUG " (" PLUGIN_BUILDSYS ")"
#endif
#else
#ifdef _DEBUG
#define PLUGIN_DEBUG " (Debug)"
#else
#define PLUGIN_DEBUG ""
#endif
#endif

#define PLUGIN_RELEASE " v0.70 "
#define PLUGIN_BUILD "WIP 6" \
	   PLUGIN_DEBUG 

#define PLUGIN_VERSION \
"Azimer's " \
PLUGIN_NAME \
PLUGIN_RELEASE \
PLUGIN_BUILD


#ifdef ENABLEPROFILING

	extern u64 ProfileStartTimes[30];
	extern u64 ProfileTimes[30];

	inline void StartProfile (int profile) {
		u64 start;
		__asm {
			rdtsc;
			mov dword ptr [start+0], eax;
			mov dword ptr [start+4], edx;
		}
		ProfileStartTimes[profile] = start;
	}

	inline void EndProfile (int profile) {
		u64 end;
		__asm {
			rdtsc;
			mov dword ptr [end+0], eax;
			mov dword ptr [end+4], edx;
		}
		ProfileTimes[profile] = ProfileTimes[profile] + (end - ProfileStartTimes[profile]);
	}
	inline void PrintProfiles () {
		FILE *dfile = fopen ("d:\\profile.txt", "wt");
		u64 totalTimes = 0;
		for (int x = 0; x < 30; x++) {
			if (ProfileTimes[x] != 0) {
				fprintf (dfile, "Times for %i is: %08X %08X\n", x, (u32)(ProfileTimes[x] >> 32), (u32)ProfileTimes[x]);
				totalTimes += ProfileTimes[x];
			}
		}
		for (x = 0; x < 30; x++) {
			if (ProfileTimes[x] != 0) {
				fprintf (dfile, "Percent Time for %i is: %i%%\n", x, (u32)((ProfileTimes[x]*100) / totalTimes));			
			}
		}
		fclose (dfile);
	}
	inline void ClearProfiles () {
		for (int x = 0; x < 30; x++) {
			ProfileTimes[x] = 0;
		}
	}
#else
#	define StartProfile(profile) //
#	define EndProfile(profile) //
#	define PrintProfiles() //
#	define ClearProfiles()//
#endif

/*
 * `strcpy` with bounds checking
 * This basically is a portable variation of Microsoft's `strcpy_s`.
 */
extern int safe_strcpy(char* dst, size_t limit, const char* src);
