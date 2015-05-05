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

#include "mytypes.h"
#include "audiohle.h"

u16 adpcmtable[0x88];

void InitInput(int *inp, int index, BYTE icode, u8 mask, u8 shifter, BYTE code, u8 srange, int vscale)
{
	inp[index] = (s16)((icode & mask) << shifter);
	if (code < srange)	
		inp[index] = ((int)((int)inp[index] * (int)vscale) >> 16);
	else 
		int catchme = 1;
}

void ADPCMFillArray(int *a, short *book1, short *book2, int l1, int l2, int *inp)
{
	for (int i = 0; i < 8; i++)
	{
		a[i] = (int)book1[i] * (int)l1;
		a[i] += (int)book2[i] * (int)l2;
		for (int i2 = 0; i2 < i; i2++)
		{
			a[i] += (int)book2[(i - 1) - i2] * inp[i2];
		}
		a[i] += (int)inp[i] * (int)2048;
	}
}

void ADPCM() { // Work in progress! :)
	BYTE Flags = (u8)(k0 >> 16) & 0xff;
	WORD Gain = (u16)(k0 & 0xffff);
	DWORD Address = (t9 & 0xffffff);// + SEGMENTS[(t9>>24)&0xf];
	WORD inPtr = 0;
	//short *out=(s16 *)(testbuff+(AudioOutBuffer>>2));
	short *out = (short *)(BufferSpace + AudioOutBuffer);
	BYTE *in = (BYTE *)(BufferSpace + AudioInBuffer);
	short count = (short)AudioCount;
	BYTE icode;
	BYTE code;
	int vscale;
	WORD index;
	WORD j;
	int a[8];
	short *book1, *book2;
	/*
	if (Address > (1024*1024*8))
	Address = (t9 & 0xffffff);
	*/
	memset(out, 0, 32);

	if (!(Flags & 0x1))
	{
		if (Flags & 0x2) {
			memcpy(out, &rdram[loopval & 0x7fffff], 32);
		}
		else {
			memcpy(out, &rdram[Address], 32);
		}
	}

	int l1 = out[15];
	int l2 = out[14];
	int inp1[8];
	int inp2[8];
	out += 16;
	while (count>0)
	{
		// the first interation through, these values are
		// either 0 in the case of A_INIT, from a special
		// area of memory in the case of A_LOOP or just
		// the values we calculated the last time

		code = BufferSpace[(AudioInBuffer + inPtr) ^ 3];
		index = code & 0xf;
		index <<= 4;									// index into the adpcm code table
		book1 = (short *)&adpcmtable[index];
		book2 = book1 + 8;
		code >>= 4;									// upper nibble is scale
		vscale = (0x8000 >> ((12 - code) - 1));			// very strange. 0x8000 would be .5 in 16:16 format
		// so this appears to be a fractional scale based
		// on the 12 based inverse of the scale value.  note
		// that this could be negative, in which case we do
		// not use the calculated vscale value... see the 
		// if(code>12) check below

		inPtr++;									// coded adpcm data lies next
		j = 0;
		while (j<8)									// loop of 8, for 8 coded nibbles from 4 bytes
			// which yields 8 short pcm values
		{
			icode = BufferSpace[(AudioInBuffer + inPtr) ^ 3];
			inPtr++;

			InitInput(inp1, j, icode, 0xf0, 8, code, 12, vscale); // this will in effect be signed
			j++;

			InitInput(inp1, j, icode, 0xf, 12, code, 12, vscale);
			j++;
		}
		j = 0;
		while (j<8)
		{
			icode = BufferSpace[(AudioInBuffer + inPtr) ^ 3];
			inPtr++;

			InitInput(inp2, j, icode, 0xf0, 8, code, 12, vscale); // this will in effect be signed
			j++;

			InitInput(inp2, j, icode, 0xf, 12, code, 12, vscale);
			j++;
		}

		ADPCMFillArray(a, book1, book2, l1, l2, inp1);

		for (j = 0; j<8; j++)
		{
			a[j ^ 1] >>= 11;
			a[j ^ 1] = pack_signed(a[j ^ 1]);
			*(out++) = a[j ^ 1];
		}
		l1 = a[6];
		l2 = a[7];

		ADPCMFillArray(a, book1, book2, l1, l2, inp2);

		for (j = 0; j<8; j++)
		{
			a[j ^ 1] >>= 11;
			a[j ^ 1] = pack_signed(a[j ^ 1]);
			*(out++) = a[j ^ 1];
		}
		l1 = a[6];
		l2 = a[7];

		count -= 32;
	}
	out -= 16;
	memcpy(&rdram[Address], out, 32);
}

void ADPCM2() { // Verified to be 100% Accurate...
	BYTE Flags = (u8)(k0 >> 16) & 0xff;
	WORD Gain = (u16)(k0 & 0xffff);
	DWORD Address = (t9 & 0xffffff);// + SEGMENTS[(t9>>24)&0xf];
	WORD inPtr = 0;
	//short *out=(s16 *)(testbuff+(AudioOutBuffer>>2));
	short *out = (short *)(BufferSpace + AudioOutBuffer);
	BYTE *in = (BYTE *)(BufferSpace + AudioInBuffer);
	short count = (short)AudioCount;
	BYTE icode;
	BYTE code;
	int vscale;
	WORD index;
	WORD j;
	int a[8];
	short *book1, *book2;

	u8 srange;
	u8 inpinc;
	u8 mask1;
	u8 mask2;
	u8 shifter;

	memset(out, 0, 32);

	if (Flags & 0x4) { // Tricky lil Zelda MM and ABI2!!! hahaha I know your secrets! :DDD
		srange = 0xE;
		inpinc = 0x5;
		mask1 = 0xC0;
		mask2 = 0x30;
		shifter = 10;
	}
	else {
		srange = 0xC;
		inpinc = 0x9;
		mask1 = 0xf0;
		mask2 = 0x0f;
		shifter = 12;
	}

	if (!(Flags & 0x1))
	{
		if (Flags & 0x2)
		{/*
		 for(int i=0;i<16;i++)
		 {
		 out[i]=*(short *)&rdram[(loopval+i*2)^2];
		 }*/
			memcpy(out, &rdram[loopval], 32);
		}
		else
		{/*
		 for(int i=0;i<16;i++)
		 {
		 out[i]=*(short *)&rdram[(Address+i*2)^2];
		 }*/
			memcpy(out, &rdram[Address], 32);
		}
	}

	int l1 = out[15];
	int l2 = out[14];
	int inp1[8];
	int inp2[8];
	out += 16;
	while (count>0) {
		code = BufferSpace[(AudioInBuffer + inPtr) ^ 3];
		index = code & 0xf;
		index <<= 4;
		book1 = (short *)&adpcmtable[index];
		book2 = book1 + 8;
		code >>= 4;
		vscale = (0x8000 >> ((srange - code) - 1));

		inPtr++;
		j = 0;

		while (j<8) {
			icode = BufferSpace[(AudioInBuffer + inPtr) ^ 3];
			inPtr++;

			InitInput(inp1, j, icode, mask1, 8, code, srange, vscale); // this will in effect be signed
			j++;

			InitInput(inp1, j, icode, mask2, shifter, code, srange, vscale);
			j++;

			if (Flags & 4) {
				InitInput(inp1, j, icode, 0xC, 12, code, 0xE, vscale); // this will in effect be signed
				j++;

				InitInput(inp1, j, icode, 0x3, 14, code, 0xE, vscale);
				j++;
			} // end flags
		} // end while



		j = 0;
		while (j<8) {
			icode = BufferSpace[(AudioInBuffer + inPtr) ^ 3];
			inPtr++;

			InitInput(inp2, j, icode, mask1, 8, code, srange, vscale);
			j++;

			InitInput(inp2, j, icode, mask2, shifter, code, srange, vscale);
			j++;

			if (Flags & 4) {
				InitInput(inp2, j, icode, 0xC, 12, code, 0xE, vscale);
				j++;

				InitInput(inp2, j, icode, 0x3, 14, code, 0xE, vscale);
				j++;
			} // end flags
		}

		ADPCMFillArray(a, book1, book2, l1, l2, inp1);

		for (j = 0; j<8; j++)
		{
			a[j ^ 1] >>= 11;
			a[j ^ 1] = pack_signed(a[j ^ 1]);
			*(out++) = a[j ^ 1];
		}
		l1 = a[6];
		l2 = a[7];

		ADPCMFillArray(a, book1, book2, l1, l2, inp2);

		for (j = 0; j<8; j++)
		{
			a[j ^ 1] >>= 11;
			a[j ^ 1] = pack_signed(a[j ^ 1]);
			*(out++) = a[j ^ 1];
		}
		l1 = a[6];
		l2 = a[7];

		count -= 32;
	}
	out -= 16;
	memcpy(&rdram[Address], out, 32);
}

void ADPCM3() { // Verified to be 100% Accurate...
	BYTE Flags = (u8)(t9 >> 0x1c) & 0xff;
	//WORD Gain=(u16)(k0&0xffff);
	DWORD Address = (k0 & 0xffffff);// + SEGMENTS[(t9>>24)&0xf];
	WORD inPtr = (t9 >> 12) & 0xf;
	//short *out=(s16 *)(testbuff+(AudioOutBuffer>>2));
	short *out = (short *)(BufferSpace + (t9 & 0xfff) + 0x4f0);
	BYTE *in = (BYTE *)(BufferSpace + ((t9 >> 12) & 0xf) + 0x4f0);
	short count = (short)((t9 >> 16) & 0xfff);
	BYTE icode;
	BYTE code;
	int vscale;
	WORD index;
	WORD j;
	int a[8];
	short *book1, *book2;

	memset(out, 0, 32);

	if (!(Flags & 0x1))
	{
		if (Flags & 0x2)
		{/*
		 for(int i=0;i<16;i++)
		 {
		 out[i]=*(short *)&rdram[(loopval+i*2)^2];
		 }*/
			memcpy(out, &rdram[loopval], 32);
		}
		else
		{/*
		 for(int i=0;i<16;i++)
		 {
		 out[i]=*(short *)&rdram[(Address+i*2)^2];
		 }*/
			memcpy(out, &rdram[Address], 32);
		}
	}

	int l1 = out[15];
	int l2 = out[14];
	int inp1[8];
	int inp2[8];
	out += 16;
	while (count>0)
	{
		// the first interation through, these values are
		// either 0 in the case of A_INIT, from a special
		// area of memory in the case of A_LOOP or just
		// the values we calculated the last time

		code = BufferSpace[(0x4f0 + inPtr) ^ 3];
		index = code & 0xf;
		index <<= 4;									// index into the adpcm code table
		book1 = (short *)&adpcmtable[index];
		book2 = book1 + 8;
		code >>= 4;									// upper nibble is scale
		vscale = (0x8000 >> ((12 - code) - 1));			// very strange. 0x8000 would be .5 in 16:16 format
		// so this appears to be a fractional scale based
		// on the 12 based inverse of the scale value.  note
		// that this could be negative, in which case we do
		// not use the calculated vscale value... see the 
		// if(code>12) check below

		inPtr++;									// coded adpcm data lies next
		j = 0;
		while (j<8)									// loop of 8, for 8 coded nibbles from 4 bytes
			// which yields 8 short pcm values
		{
			icode = BufferSpace[(0x4f0 + inPtr) ^ 3];
			inPtr++;

			InitInput(inp1, j, icode, 0xf0, 8, code, 12, vscale); // this will in effect be signed
			j++;

			InitInput(inp1, j, icode, 0xf, 12, code, 12, vscale);
			j++;
		}
		j = 0;
		while (j<8)
		{
			icode = BufferSpace[(0x4f0 + inPtr) ^ 3];
			inPtr++;

			InitInput(inp2, j, icode, 0xf0, 8, code, 12, vscale); // this will in effect be signed
			j++;

			InitInput(inp2, j, icode, 0xf, 12, code, 12, vscale);
			j++;
		}

		ADPCMFillArray(a, book1, book2, l1, l2, inp1);

		for (j = 0; j<8; j++)
		{
			a[j ^ 1] >>= 11;
			a[j ^ 1] = pack_signed(a[j ^ 1]);
			*(out++) = a[j ^ 1];
			//*(out+j)=a[j^1];
		}
		//out += 0x10;
		l1 = a[6];
		l2 = a[7];

		ADPCMFillArray(a, book1, book2, l1, l2, inp2);

		for (j = 0; j<8; j++)
		{
			a[j ^ 1] >>= 11;
			a[j ^ 1] = pack_signed(a[j ^ 1]);
			*(out++) = a[j ^ 1];
			//*(out+j+0x1f8)=a[j^1];
		}
		l1 = a[6];
		l2 = a[7];

		count -= 32;
	}
	out -= 16;
	memcpy(&rdram[Address], out, 32);
}

void LOADADPCM() { // Loads an ADPCM table - Works 100% Now 03-13-01
	u32 v0;
	v0 = (t9 & 0xffffff);// + SEGMENTS[(t9>>24)&0xf];
	//	if (v0 > (1024*1024*8))
	//		v0 = (t9 & 0xffffff);
	//	memcpy (dmem+0x4c0, rdram+v0, k0&0xffff); // Could prolly get away with not putting this in dmem
	//	assert ((k0&0xffff) <= 0x80);
	u16 *table = (u16 *)(rdram + v0);
	for (u32 x = 0; x < ((k0 & 0xffff) >> 0x4); x++) {
		adpcmtable[0x1 + (x << 3)] = table[0];
		adpcmtable[0x0 + (x << 3)] = table[1];

		adpcmtable[0x3 + (x << 3)] = table[2];
		adpcmtable[0x2 + (x << 3)] = table[3];

		adpcmtable[0x5 + (x << 3)] = table[4];
		adpcmtable[0x4 + (x << 3)] = table[5];

		adpcmtable[0x7 + (x << 3)] = table[6];
		adpcmtable[0x6 + (x << 3)] = table[7];
		table += 8;
	}
}

void LOADADPCM2() { // Loads an ADPCM table - Works 100% Now 03-13-01
	u32 v0;
	v0 = (t9 & 0xffffff);// + SEGMENTS[(t9>>24)&0xf];
	u16 *table = (u16 *)(rdram + v0); // Zelda2 Specific...

	for (u32 x = 0; x < ((k0 & 0xffff) >> 0x4); x++) {
		adpcmtable[0x1 + (x << 3)] = table[0];
		adpcmtable[0x0 + (x << 3)] = table[1];

		adpcmtable[0x3 + (x << 3)] = table[2];
		adpcmtable[0x2 + (x << 3)] = table[3];

		adpcmtable[0x5 + (x << 3)] = table[4];
		adpcmtable[0x4 + (x << 3)] = table[5];

		adpcmtable[0x7 + (x << 3)] = table[6];
		adpcmtable[0x6 + (x << 3)] = table[7];
		table += 8;
	}
}

void LOADADPCM3() { // Loads an ADPCM table - Works 100% Now 03-13-01
	u32 v0;
	v0 = (t9 & 0xffffff);
	//memcpy (dmem+0x3f0, rdram+v0, k0&0xffff);
	//assert ((k0&0xffff) <= 0x80);
	u16 *table = (u16 *)(rdram + v0);
	for (u32 x = 0; x < ((k0 & 0xffff) >> 0x4); x++) {
		adpcmtable[0x1 + (x << 3)] = table[0];
		adpcmtable[0x0 + (x << 3)] = table[1];

		adpcmtable[0x3 + (x << 3)] = table[2];
		adpcmtable[0x2 + (x << 3)] = table[3];

		adpcmtable[0x5 + (x << 3)] = table[4];
		adpcmtable[0x4 + (x << 3)] = table[5];

		adpcmtable[0x7 + (x << 3)] = table[6];
		adpcmtable[0x6 + (x << 3)] = table[7];
		table += 8;
	}
}
