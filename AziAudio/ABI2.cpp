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

extern u8 BufferSpace[0x10000];

static void SPNOOP () {
    static char buff[] = "Unknown/Unimplemented Audio Command %i in ABI 2";
    char * sprintf_offset;
    const u8 command = (unsigned char)((k0 & 0xFF000000ul) >> 24);

    sprintf_offset = strchr(&buff[0], '%'); /* Overwrite "%i" with decimal. */
    *(sprintf_offset + 0) = '0' + (command/10 % 10);
    *(sprintf_offset + 1) = '0' + (command/ 1 % 10);
    if (sprintf_offset[0] == '0')
        sprintf_offset[0] = ' '; /* Leading 0's may confuse decimal w/ octal. */
    MessageBox (NULL, buff, "Audio HLE Error", MB_OK);
}
extern u16 AudioInBuffer;		// 0x0000(T8)
extern u16 AudioOutBuffer;		// 0x0002(T8)
extern u16 AudioCount;			// 0x0004(T8)
extern u32 loopval;			// 0x0010(T8)
extern u32 SEGMENTS[0x10];

extern u16 adpcmtable[0x88];

extern u16 ResampleLUT [0x200];

bool isMKABI = false;
bool isZeldaABI = false;

void LOADADPCM2 () { // Loads an ADPCM table - Works 100% Now 03-13-01
	u32 v0;
	v0 = (t9 & 0xffffff);// + SEGMENTS[(t9>>24)&0xf];
	u16 *table = (u16 *)(rdram+v0); // Zelda2 Specific...

	for (u32 x = 0; x < ((k0&0xffff)>>0x4); x++) {
		adpcmtable[0x1+(x<<3)] = table[0];
		adpcmtable[0x0+(x<<3)] = table[1];

		adpcmtable[0x3+(x<<3)] = table[2];
		adpcmtable[0x2+(x<<3)] = table[3];

		adpcmtable[0x5+(x<<3)] = table[4];
		adpcmtable[0x4+(x<<3)] = table[5];

		adpcmtable[0x7+(x<<3)] = table[6];
		adpcmtable[0x6+(x<<3)] = table[7];
		table += 8;
	}
}

void SETLOOP2 () {
	loopval = t9 & 0xffffff; // No segment?
}

void SETBUFF2 () {
	AudioInBuffer   = u16(k0);			 // 0x00
	AudioOutBuffer	= u16((t9 >> 0x10)); // 0x02
	AudioCount		= u16(t9);			 // 0x04
}

void ADPCM2 () { // Verified to be 100% Accurate...
	BYTE Flags=(u8)(k0>>16)&0xff;
	WORD Gain=(u16)(k0&0xffff);
	DWORD Address=(t9 & 0xffffff);// + SEGMENTS[(t9>>24)&0xf];
	WORD inPtr=0;
	//short *out=(s16 *)(testbuff+(AudioOutBuffer>>2));
	short *out=(short *)(BufferSpace+AudioOutBuffer);
	BYTE *in=(BYTE *)(BufferSpace+AudioInBuffer);
	short count=(short)AudioCount;
	BYTE icode;
	BYTE code;
	int vscale;
	WORD index;
	WORD j;
	int a[8];
	short *book1,*book2;

	u8 srange;
	u8 inpinc;
	u8 mask1;
	u8 mask2;
	u8 shifter;

	memset(out,0,32);

	if (Flags & 0x4) { // Tricky lil Zelda MM and ABI2!!! hahaha I know your secrets! :DDD
		srange = 0xE;
		inpinc = 0x5;
		mask1 = 0xC0;
		mask2 = 0x30;
		shifter = 10;
	} else {
		srange = 0xC;
		inpinc = 0x9;
		mask1 = 0xf0;
		mask2 = 0x0f;
		shifter = 12;
	}

	if(!(Flags&0x1))
	{
		if(Flags&0x2)
		{/*
			for(int i=0;i<16;i++)
			{
				out[i]=*(short *)&rdram[(loopval+i*2)^2];
			}*/
			memcpy(out,&rdram[loopval],32);
		}
		else
		{/*
			for(int i=0;i<16;i++)
			{
				out[i]=*(short *)&rdram[(Address+i*2)^2];
			}*/
			memcpy(out,&rdram[Address],32);
		}
	}

	int l1=out[15];
	int l2=out[14];
	int inp1[8];
	int inp2[8];
	out+=16;
	while(count>0) {
		code=BufferSpace[(AudioInBuffer+inPtr)^3];
		index=code&0xf;
		index<<=4;
		book1=(short *)&adpcmtable[index];
		book2=book1+8;
		code>>=4;
		vscale=(0x8000>>((srange-code)-1));
		
		inPtr++;
		j=0;

		while(j<8) {
			icode=BufferSpace[(AudioInBuffer+inPtr)^3];
			inPtr++;

			inp1[j]=(s16)((icode&mask1) << 8);			// this will in effect be signed
			if(code<srange) inp1[j]=((int)((int)inp1[j]*(int)vscale)>>16);
			else int catchme=1;
			j++;

			inp1[j]=(s16)((icode&mask2)<<shifter);
			if(code<srange)	inp1[j]=((int)((int)inp1[j]*(int)vscale)>>16);
			else int catchme=1;
			j++;

			if (Flags & 4) {
				inp1[j]=(s16)((icode&0xC) << 12);			// this will in effect be signed
				if(code < 0xE) inp1[j]=((int)((int)inp1[j]*(int)vscale)>>16);
				else int catchme=1;
				j++;

				inp1[j]=(s16)((icode&0x3) << 14);
				if(code < 0xE) inp1[j]=((int)((int)inp1[j]*(int)vscale)>>16);
				else int catchme=1;
				j++;
			} // end flags
		} // end while



		j=0;
		while(j<8) {
			icode=BufferSpace[(AudioInBuffer+inPtr)^3];
			inPtr++;

			inp2[j]=(s16)((icode&mask1) << 8);
			if(code<srange) inp2[j]=((int)((int)inp2[j]*(int)vscale)>>16);
			else int catchme=1;
			j++;

			inp2[j]=(s16)((icode&mask2)<<shifter);
			if(code<srange)	inp2[j]=((int)((int)inp2[j]*(int)vscale)>>16);
			else int catchme=1;
			j++;

			if (Flags & 4) {
				inp2[j]=(s16)((icode&0xC) << 12);
				if(code < 0xE) inp2[j]=((int)((int)inp2[j]*(int)vscale)>>16);
				else int catchme=1;
				j++;

				inp2[j]=(s16)((icode&0x3) << 14);
				if(code < 0xE) inp2[j]=((int)((int)inp2[j]*(int)vscale)>>16);
				else int catchme=1;
				j++;
			} // end flags
		}

		a[0]= (int)book1[0]*(int)l1;
		a[0]+=(int)book2[0]*(int)l2;
		a[0]+=(int)inp1[0]*(int)2048;

		a[1] =(int)book1[1]*(int)l1;
		a[1]+=(int)book2[1]*(int)l2;
		a[1]+=(int)book2[0]*inp1[0];
		a[1]+=(int)inp1[1]*(int)2048;

		a[2] =(int)book1[2]*(int)l1;
		a[2]+=(int)book2[2]*(int)l2;
		a[2]+=(int)book2[1]*inp1[0];
		a[2]+=(int)book2[0]*inp1[1];
		a[2]+=(int)inp1[2]*(int)2048;

		a[3] =(int)book1[3]*(int)l1;
		a[3]+=(int)book2[3]*(int)l2;
		a[3]+=(int)book2[2]*inp1[0];
		a[3]+=(int)book2[1]*inp1[1];
		a[3]+=(int)book2[0]*inp1[2];
		a[3]+=(int)inp1[3]*(int)2048;

		a[4] =(int)book1[4]*(int)l1;
		a[4]+=(int)book2[4]*(int)l2;
		a[4]+=(int)book2[3]*inp1[0];
		a[4]+=(int)book2[2]*inp1[1];
		a[4]+=(int)book2[1]*inp1[2];
		a[4]+=(int)book2[0]*inp1[3];
		a[4]+=(int)inp1[4]*(int)2048;

		a[5] =(int)book1[5]*(int)l1;
		a[5]+=(int)book2[5]*(int)l2;
		a[5]+=(int)book2[4]*inp1[0];
		a[5]+=(int)book2[3]*inp1[1];
		a[5]+=(int)book2[2]*inp1[2];
		a[5]+=(int)book2[1]*inp1[3];
		a[5]+=(int)book2[0]*inp1[4];
		a[5]+=(int)inp1[5]*(int)2048;

		a[6] =(int)book1[6]*(int)l1;
		a[6]+=(int)book2[6]*(int)l2;
		a[6]+=(int)book2[5]*inp1[0];
		a[6]+=(int)book2[4]*inp1[1];
		a[6]+=(int)book2[3]*inp1[2];
		a[6]+=(int)book2[2]*inp1[3];
		a[6]+=(int)book2[1]*inp1[4];
		a[6]+=(int)book2[0]*inp1[5];
		a[6]+=(int)inp1[6]*(int)2048;

		a[7] =(int)book1[7]*(int)l1;
		a[7]+=(int)book2[7]*(int)l2;
		a[7]+=(int)book2[6]*inp1[0];
		a[7]+=(int)book2[5]*inp1[1];
		a[7]+=(int)book2[4]*inp1[2];
		a[7]+=(int)book2[3]*inp1[3];
		a[7]+=(int)book2[2]*inp1[4];
		a[7]+=(int)book2[1]*inp1[5];
		a[7]+=(int)book2[0]*inp1[6];
		a[7]+=(int)inp1[7]*(int)2048;

		for(j=0;j<8;j++)
		{
			a[j^1]>>=11;
			if(a[j^1]>32767) a[j^1]=32767;
			else if(a[j^1]<-32768) a[j^1]=-32768;
			*(out++)=a[j^1];
		}
		l1=a[6];
		l2=a[7];

		a[0]= (int)book1[0]*(int)l1;
		a[0]+=(int)book2[0]*(int)l2;
		a[0]+=(int)inp2[0]*(int)2048;

		a[1] =(int)book1[1]*(int)l1;
		a[1]+=(int)book2[1]*(int)l2;
		a[1]+=(int)book2[0]*inp2[0];
		a[1]+=(int)inp2[1]*(int)2048;

		a[2] =(int)book1[2]*(int)l1;
		a[2]+=(int)book2[2]*(int)l2;
		a[2]+=(int)book2[1]*inp2[0];
		a[2]+=(int)book2[0]*inp2[1];
		a[2]+=(int)inp2[2]*(int)2048;

		a[3] =(int)book1[3]*(int)l1;
		a[3]+=(int)book2[3]*(int)l2;
		a[3]+=(int)book2[2]*inp2[0];
		a[3]+=(int)book2[1]*inp2[1];
		a[3]+=(int)book2[0]*inp2[2];
		a[3]+=(int)inp2[3]*(int)2048;

		a[4] =(int)book1[4]*(int)l1;
		a[4]+=(int)book2[4]*(int)l2;
		a[4]+=(int)book2[3]*inp2[0];
		a[4]+=(int)book2[2]*inp2[1];
		a[4]+=(int)book2[1]*inp2[2];
		a[4]+=(int)book2[0]*inp2[3];
		a[4]+=(int)inp2[4]*(int)2048;

		a[5] =(int)book1[5]*(int)l1;
		a[5]+=(int)book2[5]*(int)l2;
		a[5]+=(int)book2[4]*inp2[0];
		a[5]+=(int)book2[3]*inp2[1];
		a[5]+=(int)book2[2]*inp2[2];
		a[5]+=(int)book2[1]*inp2[3];
		a[5]+=(int)book2[0]*inp2[4];
		a[5]+=(int)inp2[5]*(int)2048;

		a[6] =(int)book1[6]*(int)l1;
		a[6]+=(int)book2[6]*(int)l2;
		a[6]+=(int)book2[5]*inp2[0];
		a[6]+=(int)book2[4]*inp2[1];
		a[6]+=(int)book2[3]*inp2[2];
		a[6]+=(int)book2[2]*inp2[3];
		a[6]+=(int)book2[1]*inp2[4];
		a[6]+=(int)book2[0]*inp2[5];
		a[6]+=(int)inp2[6]*(int)2048;

		a[7] =(int)book1[7]*(int)l1;
		a[7]+=(int)book2[7]*(int)l2;
		a[7]+=(int)book2[6]*inp2[0];
		a[7]+=(int)book2[5]*inp2[1];
		a[7]+=(int)book2[4]*inp2[2];
		a[7]+=(int)book2[3]*inp2[3];
		a[7]+=(int)book2[2]*inp2[4];
		a[7]+=(int)book2[1]*inp2[5];
		a[7]+=(int)book2[0]*inp2[6];
		a[7]+=(int)inp2[7]*(int)2048;

		for(j=0;j<8;j++)
		{
			a[j^1]>>=11;
			if(a[j^1]>32767) a[j^1]=32767;
			else if(a[j^1]<-32768) a[j^1]=-32768;
			*(out++)=a[j^1];
		}
		l1=a[6];
		l2=a[7];

		count-=32;
	}
	out-=16;
	memcpy(&rdram[Address],out,32);
}

void CLEARBUFF2 () {
	u16 addr = (u16)(k0 & 0xffff);
	u16 count = (u16)(t9 & 0xffff);
	if (count > 0)
		memset(BufferSpace+addr, 0, count);
}

void LOADBUFF2 () { // Needs accuracy verification...
	u32 v0;
	u32 cnt = (((k0 >> 0xC)+3)&0xFFC);
	v0 = (t9 & 0xfffffc);// + SEGMENTS[(t9>>24)&0xf];
	memcpy (BufferSpace+(k0&0xfffc), rdram+v0, (cnt+3)&0xFFFC);
}

void SAVEBUFF2 () { // Needs accuracy verification...
	u32 v0;
	u32 cnt = (((k0 >> 0xC)+3)&0xFFC);
	v0 = (t9 & 0xfffffc);// + SEGMENTS[(t9>>24)&0xf];
	memcpy (rdram+v0, BufferSpace+(k0&0xfffc), (cnt+3)&0xFFFC);
}


void MIXER2 () { // Needs accuracy verification...
	u16 dmemin  = (u16)(t9 >> 0x10);
	u16 dmemout = (u16)(t9 & 0xFFFF);
	u32 count   = ((k0 >> 12) & 0xFF0);
	s32 gain    = (s16)(k0 & 0xFFFF)*2;
	s32 temp;

	for (u32 x=0; x < count; x+=2) { // I think I can do this a lot easier 

		temp = (*(s16 *)(BufferSpace+dmemin+x) * gain) >> 16;
		temp += *(s16 *)(BufferSpace+dmemout+x);
			
		if ((s32)temp > 32767) 
			temp = 32767;
		if ((s32)temp < -32768) 
			temp = -32768;

		*(u16 *)(BufferSpace+dmemout+x) = (u16)(temp & 0xFFFF);
	}
}


void RESAMPLE2 () {
	BYTE Flags=(u8)((k0>>16)&0xff);
	DWORD Pitch=((k0&0xffff))<<1;
	u32 addy = (t9 & 0xffffff);// + SEGMENTS[(t9>>24)&0xf];
	DWORD Accum=0;
	DWORD location;
	s16 *lut;
	short *dst;
	s16 *src;
	dst=(short *)(BufferSpace);
	src=(s16 *)(BufferSpace);
	u32 srcPtr=(AudioInBuffer/2);
	u32 dstPtr=(AudioOutBuffer/2);
	s32 temp;
	s32 accum;

	if (addy > (1024*1024*8))
		addy = (t9 & 0xffffff);

	srcPtr -= 4;

	if ((Flags & 0x1) == 0) {	
		for (int x=0; x < 4; x++) //memcpy (src+srcPtr, rdram+addy, 0x8);
			src[(srcPtr+x)^1] = ((u16 *)rdram)[((addy/2)+x)^1];
		Accum = *(u16 *)(rdram+addy+10);
	} else {
		for (int x=0; x < 4; x++)
			src[(srcPtr+x)^1] = 0;//*(u16 *)(rdram+((addy+x)^2));
	}

//	if ((Flags & 0x2))
//		__asm int 3;

	for(int i=0;i < ((AudioCount+0xf)&0xFFF0)/2;i++)	{
		location = (((Accum * 0x40) >> 0x10) * 8);
		//location = (Accum >> 0xa) << 0x3;
		lut = (s16 *)(((u8 *)ResampleLUT) + location);

		temp =  ((s32)*(s16*)(src+((srcPtr+0)^1))*((s32)((s16)lut[0])));
		accum = (s32)(temp >> 15);

		temp = ((s32)*(s16*)(src+((srcPtr+1)^1))*((s32)((s16)lut[1])));
		accum += (s32)(temp >> 15);

		temp = ((s32)*(s16*)(src+((srcPtr+2)^1))*((s32)((s16)lut[2])));
		accum += (s32)(temp >> 15);
		
		temp = ((s32)*(s16*)(src+((srcPtr+3)^1))*((s32)((s16)lut[3])));
		accum += (s32)(temp >> 15);

		if (accum > 32767) accum = 32767;
		if (accum < -32768) accum = -32768;

		dst[dstPtr^1] = (s16)(accum);
		dstPtr++;
		Accum += Pitch;
		srcPtr += (Accum>>16);
		Accum&=0xffff;
	}
	for (int x=0; x < 4; x++)
		((u16 *)rdram)[((addy/2)+x)^1] = src[(srcPtr+x)^1];
	*(u16 *)(rdram+addy+10) = (u16)Accum;
	//memcpy (RSWORK, src+srcPtr, 0x8);
}

void DMEMMOVE2 () { // Needs accuracy verification...
	u32 v0, v1;
	u32 cnt;
	if ((t9 & 0xffff)==0)
		return;
	v0 = (k0 & 0xFFFF);
	v1 = (t9 >> 0x10);
	//assert ((v1 & 0x3) == 0);
	//assert ((v0 & 0x3) == 0);
	u32 count = ((t9+3) & 0xfffc);
	//v0 = (v0) & 0xfffc;
	//v1 = (v1) & 0xfffc;

	//memcpy (dmem+v1, dmem+v0, count-1);
	for (cnt = 0; cnt < count; cnt++) {
		*(u8 *)(BufferSpace+((cnt+v1)^3)) = *(u8 *)(BufferSpace+((cnt+v0)^3));
	}
}

u32 t3, s5, s6;
u16 env[8];

void ENVSETUP1 () {
	u32 tmp;

	//fprintf (dfile, "ENVSETUP1: k0 = %08X, t9 = %08X\n", k0, t9);
	t3 = k0 & 0xFFFF;
	tmp	= (k0 >> 0x8) & 0xFF00;
	env[4] = (u16)tmp;
	tmp += t3;
	env[5] = (u16)tmp;
	s5 = t9 >> 0x10;
	s6 = t9 & 0xFFFF;
	//fprintf (dfile, "	t3 = %X / s5 = %X / s6 = %X / env[4] = %X / env[5] = %X\n", t3, s5, s6, env[4], env[5]);
}

void ENVSETUP2 () {
	u32 tmp;

	//fprintf (dfile, "ENVSETUP2: k0 = %08X, t9 = %08X\n", k0, t9);
	tmp = (t9 >> 0x10);
	env[0] = (u16)tmp;
	tmp += s5;
	env[1] = (u16)tmp;
	tmp = t9 & 0xffff;
	env[2] = (u16)tmp;
	tmp += s6;
	env[3] = (u16)tmp;
	//fprintf (dfile, "	env[0] = %X / env[1] = %X / env[2] = %X / env[3] = %X\n", env[0], env[1], env[2], env[3]);
}

void ENVMIXER2 () {
	//fprintf (dfile, "ENVMIXER: k0 = %08X, t9 = %08X\n", k0, t9);

	s16 *bufft6, *bufft7, *buffs0, *buffs1;
	s16 *buffs3;
	s32 count;
	u32 adder;
	int x;

	s16 vec9, vec10;

	s16 v2[8];

	//__asm int 3;

	buffs3 = (s16 *)(BufferSpace + ((k0 >> 0x0c)&0x0ff0));
	bufft6 = (s16 *)(BufferSpace + ((t9 >> 0x14)&0x0ff0));
	bufft7 = (s16 *)(BufferSpace + ((t9 >> 0x0c)&0x0ff0));
	buffs0 = (s16 *)(BufferSpace + ((t9 >> 0x04)&0x0ff0));
	buffs1 = (s16 *)(BufferSpace + ((t9 << 0x04)&0x0ff0));


	v2[0] = 0 - (s16)((k0 & 0x2) >> 1);
	v2[1] = 0 - (s16)((k0 & 0x1));
	v2[2] = 0 - (s16)((k0 & 0x8) >> 1);
	v2[3] = 0 - (s16)((k0 & 0x4) >> 1);

	count = (k0 >> 8) & 0xff;

	if (!isMKABI) {
		s5 *= 2; s6 *= 2; t3 *= 2;
		adder = 0x10;
	} else {
		k0 = 0;
		adder = 0x8;
		t3 = 0;
	}


	while (count > 0) {
		int temp;
		for (x=0; x < 0x8; x++) {
			vec9  = (s16)(((s32)buffs3[x^1] * (u32)env[0]) >> 0x10) ^ v2[0];
			vec10 = (s16)(((s32)buffs3[x^1] * (u32)env[2]) >> 0x10) ^ v2[1];
			temp = bufft6[x^1] + vec9;
			if (temp > 32767)  temp = 32767; if (temp < -32768) temp = -32768;
			bufft6[x^1] = temp;
			temp = bufft7[x^1] + vec10;
			if (temp > 32767)  temp = 32767; if (temp < -32768) temp = -32768;
			bufft7[x^1] = temp;
			vec9  = (s16)(((s32)vec9  * (u32)env[4]) >> 0x10) ^ v2[2];
			vec10 = (s16)(((s32)vec10 * (u32)env[4]) >> 0x10) ^ v2[3];
			if (k0 & 0x10) {
				temp = buffs0[x^1] + vec10;
				if (temp > 32767)  temp = 32767; if (temp < -32768) temp = -32768;
				buffs0[x^1] = temp;
				temp = buffs1[x^1] + vec9;
				if (temp > 32767)  temp = 32767; if (temp < -32768) temp = -32768;
				buffs1[x^1] = temp;
			} else {
				temp = buffs0[x^1] + vec9;
				if (temp > 32767)  temp = 32767; if (temp < -32768) temp = -32768;
				buffs0[x^1] = temp;
				temp = buffs1[x^1] + vec10;
				if (temp > 32767)  temp = 32767; if (temp < -32768) temp = -32768;
				buffs1[x^1] = temp;
			}
		}

		if (!isMKABI)
		for (x=0x8; x < 0x10; x++) {
			vec9  = (s16)(((s32)buffs3[x^1] * (u32)env[1]) >> 0x10) ^ v2[0];
			vec10 = (s16)(((s32)buffs3[x^1] * (u32)env[3]) >> 0x10) ^ v2[1];
			temp = bufft6[x^1] + vec9;
			if (temp > 32767)  temp = 32767; if (temp < -32768) temp = -32768;
			bufft6[x^1] = temp;
			temp = bufft7[x^1] + vec10;
			if (temp > 32767)  temp = 32767; if (temp < -32768) temp = -32768;
			bufft7[x^1] = temp;
			vec9  = (s16)(((s32)vec9  * (u32)env[5]) >> 0x10) ^ v2[2];
			vec10 = (s16)(((s32)vec10 * (u32)env[5]) >> 0x10) ^ v2[3];
			if (k0 & 0x10) {
				temp = buffs0[x^1] + vec10;
				if (temp > 32767)  temp = 32767; if (temp < -32768) temp = -32768;
				buffs0[x^1] = temp;
				temp = buffs1[x^1] + vec9;
				if (temp > 32767)  temp = 32767; if (temp < -32768) temp = -32768;
				buffs1[x^1] = temp;
			} else {
				temp = buffs0[x^1] + vec9;
				if (temp > 32767)  temp = 32767; if (temp < -32768) temp = -32768;
				buffs0[x^1] = temp;
				temp = buffs1[x^1] + vec10;
				if (temp > 32767)  temp = 32767; if (temp < -32768) temp = -32768;
				buffs1[x^1] = temp;
			}
		}
		bufft6 += adder; bufft7 += adder;
		buffs0 += adder; buffs1 += adder;
		buffs3 += adder; count  -= adder;
		env[0] += (u16)s5; env[1] += (u16)s5;
		env[2] += (u16)s6; env[3] += (u16)s6;
		env[4] += (u16)t3; env[5] += (u16)t3;
	}
}

void DUPLICATE2() {
	WORD Count = (k0 >> 16) & 0xff;
	WORD In  = k0&0xffff;
	WORD Out = (t9>>16);

	WORD buff[64];
	
	memcpy(buff,BufferSpace+In,128);

	while(Count) {
		memcpy(BufferSpace+Out,buff,128);
		Out+=128;
		Count--;
	}
}
/*
void INTERL2 () { // Make your own...
	short Count = k0 & 0xffff;
	WORD  Out   = t9 & 0xffff;
	WORD In     = (t9 >> 16);

	short *src,*dst,tmp;
	src=(short *)&BufferSpace[In];
	dst=(short *)&BufferSpace[Out];
	while(Count)
	{
		*(dst++)=*(src++);
		src++;
		*(dst++)=*(src++);
		src++;
		*(dst++)=*(src++);
		src++;
		*(dst++)=*(src++);
		src++;
		*(dst++)=*(src++);
		src++;
		*(dst++)=*(src++);
		src++;
		*(dst++)=*(src++);
		src++;
		*(dst++)=*(src++);
		src++;
		Count-=8;
	}
}
*/

void INTERL2 () {
	short Count = k0 & 0xffff;
	WORD  Out   = t9 & 0xffff;
	WORD In     = (t9 >> 16);

	BYTE *src,*dst;
	src=(BYTE *)(BufferSpace);//[In];
	dst=(BYTE *)(BufferSpace);//[Out];
	while(Count) {
		*(short *)(dst+(Out^3)) = *(short *)(src+(In^3));
		Out += 2;
		In  += 4;
		Count--;
	}
}

void INTERLEAVE2 () { // Needs accuracy verification...
	u32 inL, inR;
	u16 *outbuff;
	u16 *inSrcR;
	u16 *inSrcL;
	u16 Left, Right;
	u32 count;
	count   = ((k0 >> 12) & 0xFF0);
	if (count == 0) {
		outbuff = (u16 *)(AudioOutBuffer+BufferSpace);
		count = AudioCount;
	} else {
		outbuff = (u16 *)((k0&0xFFFF)+BufferSpace);
	}

	inR = t9 & 0xFFFF;
	inL = (t9 >> 16) & 0xFFFF;

	inSrcR = (u16 *)(BufferSpace+inR);
	inSrcL = (u16 *)(BufferSpace+inL);

	for (u32 x = 0; x < (count/4); x++) {
		Left=*(inSrcL++);
		Right=*(inSrcR++);

		*(outbuff++)=*(inSrcR++);
		*(outbuff++)=*(inSrcL++);
		*(outbuff++)=(u16)Right;
		*(outbuff++)=(u16)Left;
	}
}

void ADDMIXER () {
	short Count   = (k0 >> 12) & 0x00ff0;
	u16 InBuffer  = (t9 >> 16);
	u16 OutBuffer = t9 & 0xffff;

	s16 *inp, *outp;
	s32 temp;
	inp  = (s16 *)(BufferSpace + InBuffer);
	outp = (s16 *)(BufferSpace + OutBuffer);
	for (int cntr = 0; cntr < Count; cntr+=2) {
		temp = *outp + *inp;
		if (temp > 32767)  temp = 32767; if (temp < -32768) temp = -32768;
		outp++;	inp++;
	}
}

void HILOGAIN () {
	u16 cnt = k0 & 0xffff;
	u16 out = (t9 >> 16) & 0xffff;
	s16 hi  = (s16)((k0 >> 4) & 0xf000);
	u16 lo  = (k0 >> 20) & 0xf;
	s16 *src;

	src = (s16 *)(BufferSpace+out);
	s32 tmp, val;

	while(cnt) {
		val = (s32)*src;
		//tmp = ((val * (s32)hi) + ((u64)(val * lo) << 16) >> 16);
		tmp = ((val * (s32)hi) >> 16) + (u32)(val * lo);
		if ((s32)tmp > 32767) tmp = 32767;
		else if ((s32)tmp < -32768) tmp = -32768;
		*src = (s16)tmp;
		src++;
		cnt -= 2;
	}
}

void FILTER2 () {
			int x;
			static int cnt = 0;
			static s16 *lutt6;
			static s16 *lutt5;
			u8 *save = (rdram+(t9&0xFFFFFF));
			u8 t4 = (u8)((k0 >> 0x10) & 0xFF);

			if (t4 > 1) { // Then set the cnt variable
				cnt = (k0 & 0xFFFF);
				lutt6 = (s16 *)save;
//				memcpy (dmem+0xFE0, rdram+(t9&0xFFFFFF), 0x10);
				return;
			}

			if (t4 == 0) {
//				memcpy (dmem+0xFB0, rdram+(t9&0xFFFFFF), 0x20);
				lutt5 = (short *)(save+0x10);
			}

			lutt5 = (short *)(save+0x10);

//			lutt5 = (short *)(dmem + 0xFC0);
//			lutt6 = (short *)(dmem + 0xFE0);
			for (int x = 0; x < 8; x++) {
				s32 a;
				a = (lutt5[x] + lutt6[x]) >> 1;
				lutt5[x] = lutt6[x] = (short)a;
			}
			short *inp1, *inp2; 
			s32 out1[8];
			s16 outbuff[0x3c0], *outp;
			u32 inPtr = (u32)(k0&0xffff);
			inp1 = (short *)(save);
			outp = outbuff;
			inp2 = (short *)(BufferSpace+inPtr);
			for (x = 0; x < cnt; x+=0x10) {
				out1[1] =  inp1[0]*lutt6[6];
				out1[1] += inp1[3]*lutt6[7];
				out1[1] += inp1[2]*lutt6[4];
				out1[1] += inp1[5]*lutt6[5];
				out1[1] += inp1[4]*lutt6[2];
				out1[1] += inp1[7]*lutt6[3];
				out1[1] += inp1[6]*lutt6[0];
				out1[1] += inp2[1]*lutt6[1]; // 1

				out1[0] =  inp1[3]*lutt6[6];
				out1[0] += inp1[2]*lutt6[7];
				out1[0] += inp1[5]*lutt6[4];
				out1[0] += inp1[4]*lutt6[5];
				out1[0] += inp1[7]*lutt6[2];
				out1[0] += inp1[6]*lutt6[3];
				out1[0] += inp2[1]*lutt6[0];
				out1[0] += inp2[0]*lutt6[1];

				out1[3] =  inp1[2]*lutt6[6];
				out1[3] += inp1[5]*lutt6[7];
				out1[3] += inp1[4]*lutt6[4];
				out1[3] += inp1[7]*lutt6[5];
				out1[3] += inp1[6]*lutt6[2];
				out1[3] += inp2[1]*lutt6[3];
				out1[3] += inp2[0]*lutt6[0];
				out1[3] += inp2[3]*lutt6[1];

				out1[2] =  inp1[5]*lutt6[6];
				out1[2] += inp1[4]*lutt6[7];
				out1[2] += inp1[7]*lutt6[4];
				out1[2] += inp1[6]*lutt6[5];
				out1[2] += inp2[1]*lutt6[2];
				out1[2] += inp2[0]*lutt6[3];
				out1[2] += inp2[3]*lutt6[0];
				out1[2] += inp2[2]*lutt6[1];

				out1[5] =  inp1[4]*lutt6[6];
				out1[5] += inp1[7]*lutt6[7];
				out1[5] += inp1[6]*lutt6[4];
				out1[5] += inp2[1]*lutt6[5];
				out1[5] += inp2[0]*lutt6[2];
				out1[5] += inp2[3]*lutt6[3];
				out1[5] += inp2[2]*lutt6[0];
				out1[5] += inp2[5]*lutt6[1];

				out1[4] =  inp1[7]*lutt6[6];
				out1[4] += inp1[6]*lutt6[7];
				out1[4] += inp2[1]*lutt6[4];
				out1[4] += inp2[0]*lutt6[5];
				out1[4] += inp2[3]*lutt6[2];
				out1[4] += inp2[2]*lutt6[3];
				out1[4] += inp2[5]*lutt6[0];
				out1[4] += inp2[4]*lutt6[1];

				out1[7] =  inp1[6]*lutt6[6];
				out1[7] += inp2[1]*lutt6[7];
				out1[7] += inp2[0]*lutt6[4];
				out1[7] += inp2[3]*lutt6[5];
				out1[7] += inp2[2]*lutt6[2];
				out1[7] += inp2[5]*lutt6[3];
				out1[7] += inp2[4]*lutt6[0];
				out1[7] += inp2[7]*lutt6[1];

				out1[6] =  inp2[1]*lutt6[6];
				out1[6] += inp2[0]*lutt6[7];
				out1[6] += inp2[3]*lutt6[4];
				out1[6] += inp2[2]*lutt6[5];
				out1[6] += inp2[5]*lutt6[2];
				out1[6] += inp2[4]*lutt6[3];
				out1[6] += inp2[7]*lutt6[0];
				out1[6] += inp2[6]*lutt6[1];
				outp[1] = /*CLAMP*/(s16)((out1[1]+0x4000) >> 0xF);
				outp[0] = /*CLAMP*/(s16)((out1[0]+0x4000) >> 0xF);
				outp[3] = /*CLAMP*/(s16)((out1[3]+0x4000) >> 0xF);
				outp[2] = /*CLAMP*/(s16)((out1[2]+0x4000) >> 0xF);
				outp[5] = /*CLAMP*/(s16)((out1[5]+0x4000) >> 0xF);
				outp[4] = /*CLAMP*/(s16)((out1[4]+0x4000) >> 0xF);
				outp[7] = /*CLAMP*/(s16)((out1[7]+0x4000) >> 0xF);
				outp[6] = /*CLAMP*/(s16)((out1[6]+0x4000) >> 0xF);
				inp1 = inp2;
				inp2 += 8;
				outp += 8;
			}
//			memcpy (rdram+(t9&0xFFFFFF), dmem+0xFB0, 0x20);
			memcpy (save, inp2-8, 0x10);
			memcpy (BufferSpace+(k0&0xffff), outbuff, cnt);
}

void SEGMENT2 () {
	if (isZeldaABI) {
		FILTER2 ();
		return;
	}
	if ((k0 & 0xffffff) == 0) {
		isMKABI = true;
		//SEGMENTS[(t9>>24)&0xf] = (t9 & 0xffffff);
	} else {
		isMKABI = false;
		isZeldaABI = true;
		FILTER2 ();
	}
}

void UNKNOWN () {
}
/*
void (*ABI2[0x20])() = {
    SPNOOP, ADPCM2, CLEARBUFF2, SPNOOP, SPNOOP, RESAMPLE2, SPNOOP, SEGMENT2,
    SETBUFF2, SPNOOP, DMEMMOVE2, LOADADPCM2, MIXER2, INTERLEAVE2, HILOGAIN, SETLOOP2,
    SPNOOP, INTERL2, ENVSETUP1, ENVMIXER2, LOADBUFF2, SAVEBUFF2, ENVSETUP2, SPNOOP,
    SPNOOP, SPNOOP, SPNOOP, SPNOOP, SPNOOP, SPNOOP, SPNOOP, SPNOOP
};*/

void (*ABI2[0x20])() = {
    SPNOOP,		ADPCM2,		CLEARBUFF2, UNKNOWN, 
	ADDMIXER,	RESAMPLE2,	UNKNOWN,	SEGMENT2,
    SETBUFF2,	DUPLICATE2,	DMEMMOVE2,	LOADADPCM2, 
	MIXER2,		INTERLEAVE2,HILOGAIN,	SETLOOP2,
    SPNOOP,		INTERL2,	ENVSETUP1,	ENVMIXER2, 
	LOADBUFF2,	SAVEBUFF2,	ENVSETUP2,	SPNOOP,
    HILOGAIN,	SPNOOP,		DUPLICATE2, UNKNOWN, 
	SPNOOP,		SPNOOP,		SPNOOP,		SPNOOP
};
/*
void (*ABI2[0x20])() = {
    SPNOOP , ADPCM2, CLEARBUFF2, SPNOOP, SPNOOP, RESAMPLE2  , SPNOOP  , SEGMENT2,
    SETBUFF2 , DUPLICATE2, DMEMMOVE2, LOADADPCM2, MIXER2, INTERLEAVE2, SPNOOP, SETLOOP2,
    SPNOOP, INTERL2 , ENVSETUP1, ENVMIXER2, LOADBUFF2, SAVEBUFF2, ENVSETUP2, SPNOOP,
    SPNOOP , SPNOOP, SPNOOP , SPNOOP    , SPNOOP  , SPNOOP    , SPNOOP  , SPNOOP
};
/* NOTES:

  FILTER/SEGMENT - Still needs to be finished up... add FILTER?
  UNKNOWWN #27	 - Is this worth doing?  Looks like a pain in the ass just for WaveRace64
*/