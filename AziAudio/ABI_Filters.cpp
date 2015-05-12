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
static void packed_multiply_accumulate(pi32 acc, pi16 vs, pi16 vt, int offset)
{
	i32 result;
	register int i;

	result = 0;
	for (i = 0; i < 8; i++)
		result += (s32)vs[i + offset] * (s32)vt[i];
	*(acc) = result;
	return;
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
	swap_elements(&inputs_matrix[8]);
	for (i = 0; i < 8; i++)
		inputs_matrix[15 - (i + 8)] = inp2[i];
	swap_elements(&inputs_matrix[0]);

	for (x = 0; x < cnt; x += 0x10) {
		packed_multiply_accumulate(&out1[0], &inputs_matrix[0], &lutt6[0], 6);
		packed_multiply_accumulate(&out1[1], &inputs_matrix[0], &lutt6[0], 7);
		packed_multiply_accumulate(&out1[2], &inputs_matrix[0], &lutt6[0], 4);
		packed_multiply_accumulate(&out1[3], &inputs_matrix[0], &lutt6[0], 5);
		packed_multiply_accumulate(&out1[4], &inputs_matrix[0], &lutt6[0], 2);
		packed_multiply_accumulate(&out1[5], &inputs_matrix[0], &lutt6[0], 3);
		packed_multiply_accumulate(&out1[6], &inputs_matrix[0], &lutt6[0], 0);
		packed_multiply_accumulate(&out1[7], &inputs_matrix[0], &lutt6[0], 1);

		for (i = 0; i < 8; i++)
			outp[i] = (out1[i] + 0x4000) >> 15; /* fractional round and shift */
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
		swap_elements(&inputs_matrix[0]);
		swap_elements(&inputs_matrix[8]);
	}
	//			memcpy (rdram+(t9&0xFFFFFF), dmem+0xFB0, 0x20);
	memcpy(save, inp2 - 8, 0x10);
	memcpy(BufferSpace + (k0 & 0xffff), outbuff, cnt);
}
