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

#include "audiohle.h"

/*
 * Some combination of RSP LWC2 pack-type operations and vector multiply-
 * accumulate going on here, doing some fancy matrix math from data memory.
 */
static void packed_multiply_accumulate(i32 * acc, i16 * vs, i16 * vt)
{
	i32 pre_buffer[8];
	i32 result;
#ifdef SSE2_SUPPORT
	__m128i xmm_source, xmm_target;

	xmm_source = _mm_loadu_si128((__m128i *)vs);
	xmm_target = _mm_loadu_si128((__m128i *)vt);
	xmm_source = _mm_madd_epi16(xmm_source, xmm_target);
	_mm_storeu_si128((__m128i *)pre_buffer, xmm_source);
	result = pre_buffer[0] + pre_buffer[1] + pre_buffer[2] + pre_buffer[3];
#else
	register int i;

	for (i = 0; i < 8; i++)
		pre_buffer[i] = (s32)vs[i] * (s32)vt[i];
	result = 0;
	for (i = 0; i < 8; i++)
		result += pre_buffer[i];
#endif
	*(acc) = result;
	return;
}

// rdot is borrowed from Mupen64Plus audio.c file.
s32 rdot(size_t n, const s16 *x, const s16 *y)
{
	s32 accu = 0;

	y += n;

	while (n != 0) {
		accu += *(x++) * *(--y);
		--n;
	}

	return accu;
}

void FILTER2() {
	int x, i;
	static int cnt = 0;
	static s16 *lutt6;
	static s16 *lutt5;
	u8 *save = (rdram + (t9 & 0xFFFFFF));
	u8 t4 = (u8)((k0 >> 0x10) & 0xFF);

	if (t4 > 1) { // Then set the cnt variable
		cnt = (k0 & 0xFFFF);
		lutt6 = (s16 *)save;
		//				memcpy (dmem+0xFE0, rdram+(t9&0xFFFFFF), 0x10);
		return;
	}

	if (t4 == 0) {
		//				memcpy (dmem+0xFB0, rdram+(t9&0xFFFFFF), 0x20);
		lutt5 = (s16 *)(save + 0x10);
	}

	lutt5 = (s16 *)(save + 0x10);

	//			lutt5 = (s16 *)(dmem + 0xFC0);
	//			lutt6 = (s16 *)(dmem + 0xFE0);
	for (int x = 0; x < 8; x++) {
		s32 a;
		a = (lutt5[x] + lutt6[x]) >> 1;
		lutt5[x] = lutt6[x] = (s16)a;
	}
	i16 inputs_matrix[16];
	i16* inp1;
	i16* inp2;
	s32 out1[8];
	s16 outbuff[0x3c0], *outp;
	u32 inPtr = (u32)(k0 & 0xffff);

	inp1 = (i16 *)(save);
	outp = outbuff;
	inp2 = (i16 *)(BufferSpace + inPtr);

/*
 * The first iteration has no contiguity between inp1 and inp2.
 * Every iteration thereafter, they are contiguous:  inp1 = inp2; inp2 += 8;
 */
	for (i = 0; i < 8; i++)
		inputs_matrix[15 - (i + 0)] = inp1[i];
	swap_elements(&inputs_matrix[8], &inputs_matrix[8]);
	for (i = 0; i < 8; i++)
		inputs_matrix[15 - (i + 8)] = inp2[i];
	swap_elements(&inputs_matrix[0], &inputs_matrix[0]);

	for (x = 0; x < cnt; x += 0x10) {
		packed_multiply_accumulate(&out1[0], &inputs_matrix[6], &lutt6[0]);
		packed_multiply_accumulate(&out1[1], &inputs_matrix[7], &lutt6[0]);
		packed_multiply_accumulate(&out1[2], &inputs_matrix[4], &lutt6[0]);
		packed_multiply_accumulate(&out1[3], &inputs_matrix[5], &lutt6[0]);
		packed_multiply_accumulate(&out1[4], &inputs_matrix[2], &lutt6[0]);
		packed_multiply_accumulate(&out1[5], &inputs_matrix[3], &lutt6[0]);
		packed_multiply_accumulate(&out1[6], &inputs_matrix[0], &lutt6[0]);
		packed_multiply_accumulate(&out1[7], &inputs_matrix[1], &lutt6[0]);

		for (i = 0; i < 8; i++)
			outp[i] = (s16)((out1[i] + 0x4000) >> 15); /* fractional round and shift */
#if 0
/*
 * Clamp the result to fit within the legal range of 16-bit short elements.
 * VMULF, I know, never needs this in games, because the only way for VMULF
 * to produce an out-of-range value is if audio ucode does -32768 * -32768.
 */
		for (i = 0; i < 8; i++)
			assert(outp[i] >= -32768 && outp[i] <= +32767);
		for (i = 0; i < 8; i++)
			outp[i] = pack_signed(outp[i]);
#endif
		outp += 8;

		inp1 = inp2 + 0;
		inp2 = inp2 + 8;

		for (i = 0; i < 16; i++)
			inputs_matrix[15 - i] = inp1[i];
		swap_elements(&inputs_matrix[0], &inputs_matrix[0]);
		swap_elements(&inputs_matrix[8], &inputs_matrix[8]);
	}
	//			memcpy (rdram+(t9&0xFFFFFF), dmem+0xFB0, 0x20);
	memcpy(save, inp2 - 8, 0x10);
	memcpy(BufferSpace + (k0 & 0xffff), outbuff, cnt);
}

extern u16 adpcmtable[]; //size of 0x88 * 2

// POLEF filter - Much of the code is borrowed from Mupen64Plus
// We need to refactor for SSE2 and AziAudio customizations
// as well and enable POLEF across the board
void POLEF()
{
	BYTE Flags = (u8)((k0 >> 16) & 0xff);
	WORD Gain = (u16)(k0 & 0xffff);
	DWORD Address = (t9 & 0xffffff);// + SEGMENTS[(t9>>24)&0xf];

	s16 *dst = (s16 *)(BufferSpace + AudioOutBuffer);

	const s16* const h1 = (s16 *)adpcmtable;
	s16* const h2 = (s16 *)adpcmtable + 8;

	unsigned int i;
	s16 l1, l2;
	s16 h2_before[8];
	if (AudioCount == 0) return;
	int count = (AudioCount + 15) & ~15;

	if (Flags & A_INIT) {
		l1 = 0;
		l2 = 0;
	}
	else {
		memcpy((u8 *)hleMixerWorkArea, (rdram + Address), 8);
		l1 = *(s16 *)(hleMixerWorkArea + 4);
		l2 = *(s16 *)(hleMixerWorkArea + 6);
	}

	for (i = 0; i < 8; ++i) {
		h2_before[i] = h2[i];
		h2[i] = (((s32)h2[i] * Gain) >> 14);
	}
	s16 *inp = (s16 *)(BufferSpace + AudioInBuffer);

	do
	{
		int16_t frame[8];

		for (i = 0; i < 8; ++i)
			frame[i] = inp[i];

		for (i = 0; i < 8; ++i) {
			s32 accu = frame[i] * Gain;
			accu += h1[i] * l1 + h2_before[i] * l2 + rdot(i, h2, frame);
			dst[i^1] = pack_signed(accu>>14);
		}

		l1 = dst[6 ^ 1];
		l2 = dst[7 ^ 1];

		dst += 8;
		inp += 8;
		count -= 16;
	} while (count != 0);

	*(s16 *)(hleMixerWorkArea + 4) = l1;
	*(s16 *)(hleMixerWorkArea + 6) = l2;
	memcpy((rdram + Address), (u8 *)hleMixerWorkArea, 8);
}

