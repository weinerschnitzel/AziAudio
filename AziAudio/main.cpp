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

#include <windows.h>
#include <commctrl.h>
#include "common.h"
#include "AudioSpec.h"
#ifdef USE_XAUDIO2
#include "XAudio2SoundDriver.h"
#else
#include "DirectSoundDriver.h"
#endif
#include "audiohle.h"
//#include "rsp/rsp.h"
#include <stdio.h>
#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <ios>
#include "resource.h"

using namespace std;

#if defined(XAUDIO_LIBRARIES_UNAVAILABLE) || !defined(USE_XAUDIO2)
DirectSoundDriver snd;// = AudioCode();
#else
XAudio2SoundDriver snd;// = AudioCode();
#endif

// Direct Sound selection
char DSoundDeviceName[10][100];
LPGUID DSoundGUID[10];
int DSoundCnt;
int SelectedDSound;


// RSP Test stuff

//RSP_INFO RSPInfo;


void RedirectIOToConsole();

// New Plugin Specification


// Old Plugin Specification


// Dialogs

HINSTANCE hInstance;
static bool	bAbortAiUpdate = false;


BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  // handle to DLL module
  DWORD fdwReason,     // reason for calling function
  LPVOID lpvReserved   // reserved
  ) {
	hInstance = hinstDLL;
	if (fdwReason == DLL_PROCESS_DETACH)
	{ 
		bAbortAiUpdate = true;
		Sleep(100);
	}
	
	return TRUE;
}

BOOL CALLBACK DSEnumProc(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext )
{
	HWND hDlg = (HWND)lpContext;
	safe_strcpy(DSoundDeviceName[DSoundCnt], 99, lpszDesc);
	DSoundGUID[DSoundCnt] = lpGUID;
	if (strcmp(lpszDesc, snd.configDevice) == 0)
	{
		SelectedDSound = DSoundCnt;
	}
	DSoundCnt++;
   
   return TRUE;
}


EXPORT void CALL DllAbout ( HWND hParent ){
	MessageBox (hParent, "No About yet... ", "About Box", MB_OK);
}

INT_PTR CALLBACK ConfigProc(
  HWND hDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
  ) {

	int x, temp=-1;
	switch (uMsg) {
		case WM_INITDIALOG:
			SendMessage(GetDlgItem(hDlg,IDC_DEVICE  ),CB_RESETCONTENT, 0, 0);
			for (x = 0; x < DSoundCnt; x++) {
				SendMessage(GetDlgItem(hDlg,IDC_DEVICE),CB_ADDSTRING, 0, (long)DSoundDeviceName[x]);		
			}
			SendMessage(GetDlgItem(hDlg,IDC_DEVICE), CB_SETCURSEL, SelectedDSound, 0);
			SendMessage(GetDlgItem(hDlg,IDC_OLDSYNC)  ,BM_SETCHECK, snd.configForceSync?BST_CHECKED:BST_UNCHECKED,0);
			SendMessage(GetDlgItem(hDlg,IDC_AUDIOSYNC),BM_SETCHECK, snd.configSyncAudio?BST_CHECKED:BST_UNCHECKED,0);
			SendMessage(GetDlgItem(hDlg,IDC_AI       ),BM_SETCHECK, snd.configAIEmulation?BST_CHECKED:BST_UNCHECKED,0);
			SendMessage(GetDlgItem(hDlg,IDC_VOLUME   ),TBM_SETRANGEMIN, FALSE, 0);
			SendMessage(GetDlgItem(hDlg,IDC_VOLUME   ),TBM_SETRANGEMAX, FALSE, 100);		
			SendMessage(GetDlgItem(hDlg,IDC_MUTE     ),BM_SETCHECK, snd.configMute?BST_CHECKED:BST_UNCHECKED,0);
			if (snd.configMute) SendMessage(GetDlgItem(hDlg,IDC_VOLUME   ),TBM_SETPOS, FALSE, 100);
			else SendMessage(GetDlgItem(hDlg,IDC_VOLUME   ),TBM_SETPOS, FALSE, snd.configVolume);
			SendMessage(GetDlgItem(hDlg,IDC_VOLUME   ),TBM_SETTICFREQ,20,0);
			SendMessage(GetDlgItem(hDlg,IDC_HLE      ),BM_SETCHECK, snd.configHLE?BST_CHECKED:BST_UNCHECKED,0);
			SendMessage(GetDlgItem(hDlg,IDC_RSP      ),BM_SETCHECK, snd.configRSP?BST_CHECKED:BST_UNCHECKED,0);
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					snd.configForceSync   = SendMessage(GetDlgItem(hDlg,IDC_OLDSYNC  ),BM_GETSTATE, 0,0) == BST_CHECKED?true:false;
					snd.configSyncAudio   = SendMessage(GetDlgItem(hDlg,IDC_AUDIOSYNC),BM_GETSTATE, 0,0) == BST_CHECKED?true:false;
					snd.configAIEmulation = SendMessage(GetDlgItem(hDlg,IDC_AI       ),BM_GETSTATE, 0,0) == BST_CHECKED?true:false;
					snd.configHLE = SendMessage(GetDlgItem(hDlg,IDC_HLE      ),BM_GETSTATE, 0,0) == BST_CHECKED?true:false;
					snd.configRSP = SendMessage(GetDlgItem(hDlg,IDC_RSP      ),BM_GETSTATE, 0,0) == BST_CHECKED?true:false;
					SelectedDSound = (int)SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_GETCURSEL, 0, 0);
					safe_strcpy(snd.configDevice, 99, DSoundDeviceName[SelectedDSound]);
					EndDialog(hDlg, 0);
				break;
				case IDCANCEL:
					EndDialog(hDlg, 0);
				break;
				case IDC_MUTE:
					if (IsDlgButtonChecked(hDlg, IDC_MUTE))
					{
						snd.SetVolume(100);
						snd.configMute = true;
						SendMessage(GetDlgItem(hDlg,IDC_VOLUME   ), TBM_SETPOS, TRUE, 100);
					} else {
						snd.SetVolume(snd.configVolume);
						snd.configMute = false;
						SendMessage(GetDlgItem(hDlg,IDC_VOLUME   ), TBM_SETPOS, TRUE, snd.configVolume);
					}
				break;
			}
		break;
		case WM_KEYDOWN:
		break;
		case WM_VSCROLL:
			short int userReq = LOWORD(wParam);
			if (userReq == TB_ENDTRACK || userReq == TB_THUMBTRACK)
			{
				LRESULT position;
				DWORD dwPosition;

				position = SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_GETPOS, 0, 0);
				dwPosition = (position > 100) ? 100 : (DWORD)position;
				snd.SetVolume(dwPosition);
				if (!snd.configMute)
				{
					snd.configVolume = dwPosition;
				}
				else if (dwPosition != 100)
				{
					SendMessage(GetDlgItem(hDlg,IDC_MUTE),BM_SETCHECK, BST_UNCHECKED,0);
					snd.configMute = false;
					snd.configVolume = dwPosition;
				}
				if (dwPosition == 100) 
				{
					SendMessage(GetDlgItem(hDlg,IDC_MUTE),BM_SETCHECK, BST_CHECKED,0);
					snd.configMute = true;
				}
	        }
		break;
	}

	return FALSE;

}


EXPORT void CALL DllConfig(HWND hParent)
{
#if 0
	MessageBox(hParent, "Nothing to config yet... ", "Config Box", MB_OK);
#else
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONFIG), hParent, ConfigProc);
#endif
}

EXPORT void CALL DllTest ( HWND hParent ){
	MessageBox (hParent, "Nothing to test yet... ", "Test Box", MB_OK);
}

// Initialization / Deinitalization Functions

// Note: We call CloseDLL just in case the audio plugin was already initialized...
AUDIO_INFO AudioInfo;
DWORD Dacrate = 0;

DWORD junk;
DWORD RSPRegs[10];
BOOL audioIsInitialized = FALSE;

EXPORT BOOL CALL InitiateAudio (AUDIO_INFO Audio_Info){

	//RedirectIOToConsole();
	Dacrate = 0;
	//CloseDLL ();
	DSoundCnt = 0;
	SelectedDSound = 0;
//	if ( (DirectSoundEnumerate(DSEnumProc, NULL)) != DS_OK ) { printf("Unabled to enumerate DirectSound devices\n"); }

	snd.configAIEmulation = true;
	snd.configSyncAudio   = true;
	snd.configForceSync   = false;
	snd.configMute		  = false;
	snd.configHLE		  = true;
	snd.configRSP		  = true;
	safe_strcpy(snd.configAudioLogFolder, 499, "D:\\");

	//snd.configDevice = 0;
	snd.configVolume = 0;

	memcpy (&AudioInfo, &Audio_Info, sizeof(AUDIO_INFO));
	audioIsInitialized = !snd.Initialize (AudioInfo.hwnd);
	if (audioIsInitialized == TRUE) snd.SetVolume(snd.configVolume);
/*	RSPInfo.DMEM = AudioInfo.DMEM;
	RSPInfo.IMEM = AudioInfo.IMEM;
	RSPInfo.MemoryBswaped = AudioInfo.MemoryBswaped;
	RSPInfo.RDRAM = AudioInfo.RDRAM;

	RSPInfo.MI_INTR_REG = RSPInfo.DPC_START_REG = RSPInfo.DPC_END_REG = RSPInfo.DPC_CURRENT_REG = 
		RSPInfo.DPC_STATUS_REG = RSPInfo.DPC_CLOCK_REG = RSPInfo.DPC_BUFBUSY_REG = 
		RSPInfo.DPC_PIPEBUSY_REG = RSPInfo.DPC_TMEM_REG = &junk;
	RSPInfo.SP_MEM_ADDR_REG  = &RSPRegs[0];
	RSPInfo.SP_DRAM_ADDR_REG = &RSPRegs[1];
	RSPInfo.SP_RD_LEN_REG	 = &RSPRegs[2];
	RSPInfo.SP_WR_LEN_REG	 = &RSPRegs[3];
	RSPInfo.SP_STATUS_REG	 = &RSPRegs[4];
	RSPInfo.SP_DMA_FULL_REG  = &RSPRegs[5];
	RSPInfo.SP_DMA_BUSY_REG  = &RSPRegs[6];
	RSPInfo.SP_PC_REG		 = &RSPRegs[7];
	RSPInfo.SP_SEMAPHORE_REG = &RSPRegs[8];

	InitiateRSP(RSPInfo, &junk);*/
	ChangeABI(0);
	return TRUE;
}

EXPORT void CALL CloseDLL (void){
	ChangeABI (0);
	if (audioIsInitialized == TRUE) snd.DeInitialize();
	snd.DeInitialize();
}

EXPORT void CALL GetDllInfo ( PLUGIN_INFO * PluginInfo ){
	PluginInfo->MemoryBswaped = TRUE;
	PluginInfo->NormalMemory  = FALSE;
	safe_strcpy(PluginInfo->Name, 100, PLUGIN_VERSION);
	PluginInfo->Type = PLUGIN_TYPE_AUDIO;
	PluginInfo->Version = 0x0101; // Set this to retain backwards compatibility
}

EXPORT void CALL ProcessAList(void){
	/*WINDOWINFO wi;
	if ((GetKeyState(VK_CONTROL) & GetKeyState(VK_MENU) & GetKeyState(VK_F12) & 0x100) &&
		(GetForegroundWindow() == AudioInfo.hwnd))
	{
		printf ("Opening Debugger...");
	}*/
	if (snd.configHLE) {
		HLEStart ();
	} else if (snd.configRSP) {
/*		RspClosed();
		*RSPInfo.SP_PC_REG = 0x1000;
		DoRspCycles(100);*/
	}
}

EXPORT void CALL RomOpened(void) {
	ChangeABI(0);
	snd.DeInitialize();
	Dacrate = 0;
	audioIsInitialized = !snd.Initialize(AudioInfo.hwnd);
//	RspClosed();
}

EXPORT void CALL RomClosed (void){
	ChangeABI (0);
	snd.DeInitialize();
	Dacrate = 0;
	audioIsInitialized = !snd.Initialize(AudioInfo.hwnd);
//	RspClosed();
}

EXPORT void CALL AiDacrateChanged (int  SystemType) {
	DWORD Frequency, video_clock;

	if (Dacrate == *AudioInfo.AI_DACRATE_REG)
		return;

	Dacrate = *AudioInfo.AI_DACRATE_REG & 0x00003FFF;
#ifdef _DEBUG
	if (Dacrate != *AudioInfo.AI_DACRATE_REG)
		MessageBox(
			NULL,
			"Unknown/reserved bits in AI_DACRATE_REG set.",
			"Warning",
			MB_ICONWARNING
		);
#endif
	switch (SystemType) {
		default         :  MessageBox(NULL, "Invalid SystemType.", NULL, MB_ICONERROR);
		case SYSTEM_NTSC:  video_clock = 48681812; break;
		case SYSTEM_PAL :  video_clock = 49656530; break;
		case SYSTEM_MPAL:  video_clock = 48628316; break;
	}
	Frequency = video_clock / (Dacrate + 1);
	if (audioIsInitialized == TRUE) snd.SetFrequency(Frequency);
}

EXPORT void CALL AiLenChanged (void){
	DWORD retVal; 
	if (audioIsInitialized == FALSE)
	{
		*AudioInfo.AI_STATUS_REG = AI_STATUS_DMA_BUSY;
		*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
//		AudioInfo.CheckInterrupts();
		return;
	}
	retVal = snd.AddBuffer (
		(AudioInfo.RDRAM + (*AudioInfo.AI_DRAM_ADDR_REG & 0x00FFFFF8)), 
		*AudioInfo.AI_LEN_REG & 0x3FFF8);
	//if (retVal >= *AudioInfo.AI_LEN_REG) {
	//	*AudioInfo.AI_STATUS_REG |= AI_STATUS_FIFO_FULL;
	//} else {
	//	*AudioInfo.AI_STATUS_REG |= AI_STATUS_DMA_BUSY;
	//}
	//if (retVal & SND_IS_FULL)
	//	*AudioInfo.AI_STATUS_REG |= AI_STATUS_FIFO_FULL;
	//*AudioInfo.AI_STATUS_REG |= AI_STATUS_DMA_BUSY;

	// 1: Record time of buffer
	// 2: Add Buffer to playback
	// 3: Flag empty buffer as filled
}

EXPORT DWORD CALL AiReadLength (void){
	if (audioIsInitialized == FALSE) return 0;
	*AudioInfo.AI_LEN_REG = snd.GetReadStatus ();
	return *AudioInfo.AI_LEN_REG;

	// 1: Calculate remaining buffer based on time remaining
	// 2: Return calculated remaining buffer
}

// Deprecated Functions


EXPORT void CALL AiUpdate (BOOL Wait) {
	static int intCount = 0;
	if (Wait)
	{
		if (bAbortAiUpdate == true) 
		{
			if (intCount > 10) 
				ExitThread(0);
			intCount++;
			return;
		}
		Sleep(10);
	}
	//Sleep(1);
	/*if (audioIsInitialized == TRUE)
		snd.AiUpdate(Wait);
	else if (Wait) WaitMessage();*/
	return;
		//Sleep(1);
	//snd.AiUpdate(Wait);
	/*
	if (GetAsyncKeyState(VK_OEM_PLUS) & 0x8000)
	{
		MessageBox(NULL, "Plus", "", MB_OK);
	} else if (GetAsyncKeyState(VK_OEM_MINUS) & 0x8000)
	{
		MessageBox(NULL, "Minus", "", MB_OK);
	}
	Sleep(10);
	*/
	// 1: Check to see if timer has elapsed
	// 2: If timer has elapsed, flag buffer as empty and generate AI interrupt
	// 3: Sleep(1) timer
	DWORD ticks;

	ticks = GetTickCount();	
}

static const WORD MAX_CONSOLE_LINES = 500;

void RedirectIOToConsole() {
	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;
	// allocate a console for this app
	FreeConsole ();
	if (!AllocConsole())
		return;
	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );
	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );
	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );
	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog 
	// point to console as well
	ios::sync_with_stdio();
}

int safe_strcpy(char* dst, size_t limit, const char* src)
{
#if defined(_MSC_VER)
    return strcpy_s(dst, limit, src);
#else
    size_t bytes;
    int failure;

    if (dst == NULL || src == NULL)
        return (failure = 22); /* EINVAL, from MSVC <errno.h> */

    bytes = strlen(src) + 1; /* strlen("abc") + 1 == 4 bytes */
    failure = 34; /* ERANGE, from MSVC <errno.h> */
    if (bytes > limit)
        bytes = limit;
    else
        failure = 0;

    memcpy(dst, src, bytes);
    dst[limit - 1] = '\0'; /* in case of ERANGE, may not be null-terminated */
    return (failure);
#endif
}
