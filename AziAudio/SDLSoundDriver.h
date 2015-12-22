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

#include "SoundDriver.h"

class SDLSoundDriver : public SoundDriver
{
public:
    SDLSoundDriver();
    ~SDLSoundDriver();

    Boolean Initialize();
    void DeInitialize();

#ifdef LEGACY_SOUND_DRIVER
    u32 GetReadStatus();
    u32 AddBuffer(u8* start, u32 length);
#endif

    void AiUpdate(Boolean Wait); // optional
    void StopAudio();            // stops the audio playback (as if paused)
    void StartAudio();           // starts the audio playback (as if unpaused)
    void SetFrequency(u32 Frequency);

    void SetVolume(u32 volume);
protected:
    bool dllInitialized;
};
