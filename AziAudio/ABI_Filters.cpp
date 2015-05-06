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

void FILTER2() {
	int x;
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
		lutt5 = (short *)(save + 0x10);
	}

	lutt5 = (short *)(save + 0x10);

	//			lutt5 = (short *)(dmem + 0xFC0);
	//			lutt6 = (short *)(dmem + 0xFE0);
	for (int x = 0; x < 8; x++) {
		s32 a;
		a = (lutt5[x] + lutt6[x]) >> 1;
		lutt5[x] = lutt6[x] = (short)a;
	}
	i16 inputs_matrix[16];
	pi16 inp1, inp2;
	s32 out1[8];
	s16 outbuff[0x3c0], *outp;
	u32 inPtr = (u32)(k0 & 0xffff);

	inp1 = (i16 *)(save);
	outp = outbuff;
	inp2 = (i16 *)(BufferSpace + inPtr);
	for (x = 0; x < 8; x++)
		inputs_matrix[x + 0] = inp1[x];
	for (x = 0; x < 8; x++)
		inputs_matrix[x + 8] = inp2[x];

	for (x = 0; x < cnt; x += 0x10) {
		out1[0] =
			inputs_matrix[( 9) ^ 1] * lutt6[1] +
			inputs_matrix[( 8) ^ 1] * lutt6[0] +
			inputs_matrix[( 7) ^ 1] * lutt6[3] +
			inputs_matrix[( 6) ^ 1] * lutt6[2] +
			inputs_matrix[( 5) ^ 1] * lutt6[5] +
			inputs_matrix[( 4) ^ 1] * lutt6[4] +
			inputs_matrix[( 3) ^ 1] * lutt6[7] +
			inputs_matrix[( 2) ^ 1] * lutt6[6]
		;
		out1[1] =
			inputs_matrix[( 8) ^ 1] * lutt6[1] +
			inputs_matrix[( 7) ^ 1] * lutt6[0] +
			inputs_matrix[( 6) ^ 1] * lutt6[3] +
			inputs_matrix[( 5) ^ 1] * lutt6[2] +
			inputs_matrix[( 4) ^ 1] * lutt6[5] +
			inputs_matrix[( 3) ^ 1] * lutt6[4] +
			inputs_matrix[( 2) ^ 1] * lutt6[7] +
			inputs_matrix[( 1) ^ 1] * lutt6[6]
		;
		out1[2] =
			inputs_matrix[(11) ^ 1] * lutt6[1] +
			inputs_matrix[(10) ^ 1] * lutt6[0] +
			inputs_matrix[( 9) ^ 1] * lutt6[3] +
			inputs_matrix[( 8) ^ 1] * lutt6[2] +
			inputs_matrix[( 7) ^ 1] * lutt6[5] +
			inputs_matrix[( 6) ^ 1] * lutt6[4] +
			inputs_matrix[( 5) ^ 1] * lutt6[7] +
			inputs_matrix[( 4) ^ 1] * lutt6[6]
		;
		out1[3] =
			inputs_matrix[(10) ^ 1] * lutt6[1] +
			inputs_matrix[( 9) ^ 1] * lutt6[0] +
			inputs_matrix[( 8) ^ 1] * lutt6[3] +
			inputs_matrix[( 7) ^ 1] * lutt6[2] +
			inputs_matrix[( 6) ^ 1] * lutt6[5] +
			inputs_matrix[( 5) ^ 1] * lutt6[4] +
			inputs_matrix[( 4) ^ 1] * lutt6[7] +
			inputs_matrix[( 3) ^ 1] * lutt6[6]
		;
		out1[4] =
			inputs_matrix[(13) ^ 1] * lutt6[1] +
			inputs_matrix[(12) ^ 1] * lutt6[0] +
			inputs_matrix[(11) ^ 1] * lutt6[3] +
			inputs_matrix[(10) ^ 1] * lutt6[2] +
			inputs_matrix[( 9) ^ 1] * lutt6[5] +
			inputs_matrix[( 8) ^ 1] * lutt6[4] +
			inputs_matrix[( 7) ^ 1] * lutt6[7] +
			inputs_matrix[( 6) ^ 1] * lutt6[6]
		;
		out1[5] =
			inputs_matrix[(12) ^ 1] * lutt6[1] +
			inputs_matrix[(11) ^ 1] * lutt6[0] +
			inputs_matrix[(10) ^ 1] * lutt6[3] +
			inputs_matrix[( 9) ^ 1] * lutt6[2] +
			inputs_matrix[( 8) ^ 1] * lutt6[5] +
			inputs_matrix[( 7) ^ 1] * lutt6[4] +
			inputs_matrix[( 6) ^ 1] * lutt6[7] +
			inputs_matrix[( 5) ^ 1] * lutt6[6]
		;
		out1[6] =
			inputs_matrix[(15) ^ 1] * lutt6[1] +
			inputs_matrix[(14) ^ 1] * lutt6[0] +
			inputs_matrix[(13) ^ 1] * lutt6[3] +
			inputs_matrix[(12) ^ 1] * lutt6[2] +
			inputs_matrix[(11) ^ 1] * lutt6[5] +
			inputs_matrix[(10) ^ 1] * lutt6[4] +
			inputs_matrix[( 9) ^ 1] * lutt6[7] +
			inputs_matrix[( 8) ^ 1] * lutt6[6]
		;
		out1[7] =
			inputs_matrix[(14) ^ 1] * lutt6[1] +
			inputs_matrix[(13) ^ 1] * lutt6[0] +
			inputs_matrix[(12) ^ 1] * lutt6[3] +
			inputs_matrix[(11) ^ 1] * lutt6[2] +
			inputs_matrix[(10) ^ 1] * lutt6[5] +
			inputs_matrix[( 9) ^ 1] * lutt6[4] +
			inputs_matrix[( 8) ^ 1] * lutt6[7] +
			inputs_matrix[( 7) ^ 1] * lutt6[6]
		;

		outp[0] = /*CLAMP*/(s16)((out1[0] + 0x4000) >> 0xF);
		outp[1] = /*CLAMP*/(s16)((out1[1] + 0x4000) >> 0xF);
		outp[2] = /*CLAMP*/(s16)((out1[2] + 0x4000) >> 0xF);
		outp[3] = /*CLAMP*/(s16)((out1[3] + 0x4000) >> 0xF);
		outp[4] = /*CLAMP*/(s16)((out1[4] + 0x4000) >> 0xF);
		outp[5] = /*CLAMP*/(s16)((out1[5] + 0x4000) >> 0xF);
		outp[6] = /*CLAMP*/(s16)((out1[6] + 0x4000) >> 0xF);
		outp[7] = /*CLAMP*/(s16)((out1[7] + 0x4000) >> 0xF);
		inp1 = inp2;
		inp2 += 8;
		outp += 8;
	}
	//			memcpy (rdram+(t9&0xFFFFFF), dmem+0xFB0, 0x20);
	memcpy(save, inp2 - 8, 0x10);
	memcpy(BufferSpace + (k0 & 0xffff), outbuff, cnt);
}