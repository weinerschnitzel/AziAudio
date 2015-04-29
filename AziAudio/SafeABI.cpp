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

//extern "C" { void (*CheckForHLE)( void ); }
//void HLECheck1();
//void HLECheck2();
//void HLECheck3();
//void HLECheck5();

extern "C"
{
	// MusyX HLE provided by Mupen64Plus authored by Bobby Smiles
	void ProcessMusyX_v1();
	void ProcessMusyX_v2();
}

extern void (*ABI[0x20])();
extern u32 SEGMENTS[0x10];

extern bool isMKABI;
extern bool isZeldaABI;

void smDetect () {
	u32 UCData= ((u32*)dmem)[0xFD8/4]; //TASK_UCODE_DATA
	u8  *UData = rdram+UCData;

	memset (&SEGMENTS[0], 0, 0x10*4);

	isMKABI = isZeldaABI = false;

	if (((u32*)UData)[0] != 0x1) { // Then it's either ABI 3, 4 or 5
		if (*(u32*)(UData + (0x10)) == 0x00000001) {
			ChangeABI (5); // v1
			ProcessMusyX_v1();
			return;
		}
		else {
			ChangeABI (3);
			//CheckForHLE = HLECheck3;
		}
	} else {
		if (*(u32*)(UData + (0x30)) == 0xF0000F00) { // Should be common in ABI 1 :)
			ChangeABI(1);
			//CheckForHLE = HLECheck1;
		} else if (*(u32*)(UData + (0x10)) == 0x00010010) { // v2
			ChangeABI(5);
			ProcessMusyX_v2();
			return;
		} else if (*(u32*)(UData+(0x4)) == 0x0002ffff) {
			ChangeABI (2);  //ABI 2...
			//CheckForHLE = HLECheck2;
		} else {
			ChangeABI (5);
			ProcessMusyX_v2();
			return;
			//CheckForHLE = HLECheck5;
		}
	}
	ABI[k0 >> 24](); // Do the command which was skipped :)
/*
	u32 Boot  = ((u32*)dmem)[0xFC8/4], BootLen = ((u32*)dmem)[0xFCC/4];
	u32 UC    = ((u32*)dmem)[0xFD0/4], UCLen   = ((u32*)dmem)[0xFD4/4];

	static int runonce = 0;
	static int alist=0;

	dfile = fopen ("d:\\HLEInfo.txt", "wt");

	fprintf (dfile, "--------- Audio List #%i --------\n\n", ++alist);
	fprintf (dfile, "LQV: %04X %04X %04X %04X %04X %04X %04X %04X\n", 
		((u16*)UData)[1], ((u16*)UData)[0], ((u16*)UData)[3], ((u16*)UData)[2],
		((u16*)UData)[5], ((u16*)UData)[4], ((u16*)UData)[7], ((u16*)UData)[6]);

	fprintf (dfile, "Functions: \n");
	for (int xx = 0; xx < 0x20; xx++) {
		fprintf (dfile, "%02X: %02X\n", xx, *(u16*)(UData+((0x10+(xx*2))^2)));
	}
	fclose (dfile);*/
}

void (*SafeABI[0x20])() = { // SafeMode UCode (used for Audio detection/experimentation)
    smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect,
    smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect,
    smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect,
    smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect
};
