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

/*
 * Ultra64 data types for working with RCP stuff--useful for HLE
 */
#include "my_types.h"

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

extern u32 UCData, UDataLen;

void SaveSettings( void );
void LoadSettings( void );

void RspDump ();

// ABI Functions
void ADDMIXER();
void ADPCM(); void ADPCM2(); void ADPCM3();
void CLEARBUFF(); void CLEARBUFF2(); void CLEARBUFF3();
void DMEMMOVE(); void DMEMMOVE2(); void DMEMMOVE3();
void DUPLICATE2();
void ENVMIXER(); void ENVMIXER2(); void ENVMIXER3();
void ENVSETUP1(); void ENVSETUP2();
void FILTER2();
void HILOGAIN();
void INTERL2();
void INTERLEAVE(); void INTERLEAVE2(); void INTERLEAVE3();
void LOADADPCM(); void LOADADPCM2(); void LOADADPCM3();
void LOADBUFF(); void LOADBUFF2(); void LOADBUFF3();
void MIXER(); void MIXER2(); void MIXER3();
void MP3();
void MP3ADDY();
void RESAMPLE(); void RESAMPLE2(); void RESAMPLE3();
void SAVEBUFF(); void SAVEBUFF2(); void SAVEBUFF3();
void SEGMENT(); void SEGMENT2();
void SETBUFF(); void SETBUFF2(); 
void SETLOOP(); void SETLOOP2(); void SETLOOP3();
void SETVOL(); void SETVOL3();
void SPNOOP();
void UNKNOWN();

// Buffer Space
extern u8 BufferSpace[0x10000];
extern short hleMixerWorkArea[256];
extern u32 SEGMENTS[0x10];		// 0x0320
extern u16 AudioInBuffer, AudioOutBuffer, AudioCount;
extern u16 AudioAuxA, AudioAuxC, AudioAuxE;
extern u32 loopval; // Value set by A_SETLOOP : Possible conflict with SETVOLUME???
extern bool isMKABI;
extern bool isZeldaABI;

/*
 * Include the SSE2 headers if MSVC is set to target SSE2 in code generation.
 */
#if defined(_M_IX86_FP) && (_M_IX86_FP >= 2)
#include <emmintrin.h>
#endif

/* ... or if compiled with the right preprocessor token on other compilers */
#ifdef SSE2_SUPPORT
#include <emmintrin.h>
#endif

/* The SSE1 and SSE2 headers always define these macro functions: */
#undef SSE2_SUPPORT
#if defined(_MM_SHUFFLE) && defined(_MM_SHUFFLE2)
#define SSE2_SUPPORT
#endif

#if 0
#define PREFER_MACRO_FUNCTIONS
#endif

/*
 * RSP hardware has two types of saturated arithmetic:
 *     1.  signed clamping from 32- to 16-bit elements
 *     2.  unsigned clamping from 32- to 16-bit elements
 *
 * Accumulators are 48-bit, but only 32-bit-segment intervals at one time are
 * involved in the clamp.  The upper and lower bounds for signed and unsigned
 * clamping match the ANSI C language minimum requirements for the types
 * `short` and `unsigned short`:  -32768 <= x <= +32767 and 0 <= x <= +65535.
 *
 * A "slice" is an off-set portion of the accumulator:  high, middle, or low.
 */
#ifdef PREFER_MACRO_FUNCTIONS
#define sats_over(slice)        (((slice) > +32767) ? +32767  : (slice))
#define sats_under(slice)       (((slice) < -32768) ? -32768  : (slice))
#define satu_over(slice)        (((slice) < 0x0000) ? 0x0000U : (slice))
#define satu_under(slice)       (((slice) > 0xFFFF) ? 0xFFFFU : (slice))
#else
extern INLINE s32 sats_over(s32 slice);
extern INLINE s32 sats_under(s32 slice);
extern INLINE s32 satu_over(s32 slice);
extern INLINE s32 satu_under(s32 slice);
#endif

#ifdef PREFER_MACRO_FUNCTIONS
#define pack_signed(slice)      sats_over(sats_under(slice))
#define pack_unsigned(slice)    satu_over(satu_under(slice))
#else
extern INLINE s16 pack_signed(s32 slice);
extern INLINE u16 pack_unsigned(s32 slice);
#endif

#ifdef PREFER_MACRO_FUNCTIONS
#define vsats128(vd, vs) {      \
vd[0] = pack_signed(vs[0]); vd[1] = pack_signed(vs[1]); \
vd[2] = pack_signed(vs[2]); vd[3] = pack_signed(vs[3]); }
#define vsatu128(vd, vs) {      \
vd[0] = pack_unsigned(vs[0]); vd[1] = pack_unsigned(vs[1]); \
vd[2] = pack_unsigned(vs[2]); vd[3] = pack_unsigned(vs[3]); }
#define vsats64(vd, vs) {       \
vd[0] = pack_signed(vs[0]); vd[1] = pack_signed(vs[1]); }
#define vsatu64(vd, vs) {       \
vd[0] = pack_unsigned(vs[0]); vd[1] = pack_unsigned(vs[1]); }
#else
extern INLINE void vsats128(s16* vd, s32* vs); /* Clamp vectors using SSE2. */
extern INLINE void vsatu128(u16* vd, s32* vs);
extern INLINE void vsats64 (s16* vd, s32* vs); /* Clamp vectors using MMX. */
extern INLINE void vsatu64 (u16* vd, s32* vs);
#endif

/*
 * Basically, if we can prove that, on the C implementation in question:
 *     1.  Integers are negative if all bits are set (or simply the MSB).
 *     2.  Shifting a signed integer to the right shifts in the sign bit.
 * ... then we are able to easily staticize [un-]signed clamping functions.
 */
#if (~0 < 0)
#if ((~0 & ~1) >> 1 == ~0)
#define TWOS_COMPLEMENT_NEGATION
#endif
#endif
