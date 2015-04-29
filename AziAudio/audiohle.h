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
/* Audio commands: ABI 1 */
/*
#define	A_SPNOOP				0
#define	A_ADPCM					1
#define	A_CLEARBUFF				2
#define	A_ENVMIXER				3
#define	A_LOADBUFF				4
#define	A_RESAMPLE				5
#define A_SAVEBUFF				6
#define A_SEGMENT				7
#define A_SETBUFF				8
#define A_SETVOL				9
#define A_DMEMMOVE              10
#define A_LOADADPCM             11
#define A_MIXER					12
#define A_INTERLEAVE            13
#define A_POLEF                 14
#define A_SETLOOP               15
*/
#define ACMD_SIZE               32
/*
 * Audio flags
 */

#define A_INIT			0x01
#define A_CONTINUE		0x00
#define A_LOOP          0x02
#define A_OUT           0x02
#define A_LEFT			0x02
#define	A_RIGHT			0x00
#define A_VOL			0x04
#define A_RATE			0x00
#define A_AUX			0x08
#define A_NOAUX			0x00
#define A_MAIN			0x00
#define A_MIX			0x10

//------------------------------------------------------------------------------------------

extern u32 t9, k0;

// These must be defined...
#include "AudioSpec.h"
#define dmem	AudioInfo.DMEM
#define imem	AudioInfo.IMEM
#define rdram	AudioInfo.RDRAM

// Use these functions to interface with the HLE Audio...
void HLEStart ();
void ChangeABI (int type); // type 0 = AutoDetectMode

extern u16 AudioInBuffer, AudioOutBuffer, AudioCount;
extern u16 AudioAuxA, AudioAuxC, AudioAuxE;
extern u32 loopval; // Value set by A_SETLOOP : Possible conflict with SETVOLUME???
extern u32 UCData, UDataLen;
//extern u32 SEGMENTS[0x10];

void SaveSettings( void );
void LoadSettings( void );

void RspDump ();

//extern const u16 ResampleLUT [0x200];

/*
extern u32 r0, at, v0, v1, a0, a1, a2, a3;
extern u32 t0, t1, t2, t3, t4, t5, t6, t7;
extern u32 s0, s1, s2, s3, s4, s5, s6, s7;
extern u32 t8, t9, k0, k1, gp, sp, s8, ra;
*/