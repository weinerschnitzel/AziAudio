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
#include "AudioSpec.h"

#include "SoundDriverInterface.h"
#include "SoundDriverFactory.h"

#include "audiohle.h"
//#include "rsp/rsp.h"

#include <stdio.h> // needed for configuration

#ifdef USE_PRINTF
	#include <io.h>
	#include <fcntl.h>
	#include <ios>
#endif

#include "resource.h"

using namespace std;

SoundDriverInterface *snd = NULL;

// Dialog Procedures
#if defined(_WIN32)
INT_PTR CALLBACK ConfigProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

// DirectSound selection
#ifdef _WIN32
LPGUID DSoundGUID[10];
#endif
char DSoundDeviceName[10][100];
int DSoundCnt;
int SelectedDSound;

bool bLockAddrRegister = false;
u32 LockAddrRegisterValue = 0;


#ifdef USE_PRINTF
  void RedirectIOToConsole();
#endif

HINSTANCE hInstance;

#ifdef __GNUC__
extern "C"
#endif

#ifdef _WIN32
BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  // handle to DLL module
  DWORD fdwReason,     // reason for calling function
  LPVOID lpvReserved   // reserved
  ) {
	UNREFERENCED_PARAMETER(lpvReserved);
	UNREFERENCED_PARAMETER(fdwReason);
	hInstance = hinstDLL;
	return TRUE;
}
#endif

EXPORT void CALL DllAbout(HWND hParent) {
#if defined(_WIN32) || defined(_XBOX)
	MessageBoxA(hParent, "No About yet... ", "About Box", MB_OK);
#else
	puts(PLUGIN_VERSION);
#endif
}

EXPORT void CALL DllConfig(HWND hParent)
{
#if defined(_WIN32) && !defined(_XBOX)
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONFIG), hParent, ConfigProc);
#else
	fputs("To do:  Implement saving configuration settings.\n", stderr);
#endif
}

EXPORT void CALL DllTest(HWND hParent) {
#if defined(_WIN32)
	MessageBoxA(hParent, "Nothing to test yet... ", "Test Box", MB_OK);
#else
	puts("DllTest");
#endif
}

// Initialization / Deinitalization Functions

// Note: We call CloseDLL just in case the audio plugin was already initialized...
AUDIO_INFO AudioInfo;
u32 Dacrate = 0;

// TODO: Instead of checking for an initialized state, we should default to a no-sound audio processing state and give a warning
// Boolean audioIsInitialized = FALSE;

EXPORT Boolean CALL InitiateAudio(AUDIO_INFO Audio_Info) {
	if (snd != NULL)
	{
		snd->AI_Shutdown();
		delete snd;
	}

	snd = SoundDriverFactory::CreateSoundDriver(SoundDriverType::SND_DRIVER_XA2);

#ifdef USE_PRINTF
	RedirectIOToConsole();
	dprintf("Logging to console enabled...\n");
#endif
	Dacrate = 0;
	//CloseDLL ();
	DSoundCnt = 0;
	SelectedDSound = 0;
//	if ( (DirectSoundEnumerate(DSEnumProc, NULL)) != DS_OK ) { printf("Unable to enumerate DirectSound devices\n"); }

	// TODO: Move from SoundDriver to a configuration class
	safe_strcpy(Configuration::configAudioLogFolder, 499, "D:\\");

	//Configuration::configDevice = 0;

	memcpy(&AudioInfo, &Audio_Info, sizeof(AUDIO_INFO));
	DRAM = Audio_Info.RDRAM;
	DMEM = Audio_Info.DMEM;
	IMEM = Audio_Info.IMEM;

	// Defaults:

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS) && !defined(_XBOX)
	strcpy_s(Configuration::configAudioLogFolder, 500, "D:\\");
	strcpy_s(Configuration::configDevice, 100, "");
#else
	strcpy(Configuration::configAudioLogFolder, "D:\\");
	strcpy(Configuration::configDevice, "");
#endif


	size_t file_size;
	unsigned char azicfg[4];
	FILE *file;
	file = fopen("Config/AziCfg.bin", "rb");
	if (file == NULL)
	{
		azicfg[0] = TRUE;
		azicfg[1] = FALSE;
		azicfg[2] = TRUE;
		azicfg[3] = 0; /* 0:  max volume; 100:  min volume */
	}
	else
	{
		for (file_size = 0; file_size < sizeof(azicfg); file_size++) {
			const int character = fgetc(file);
			if (character < 0 || character > 255)
				break; /* hit EOF or a disk read error */
			azicfg[file_size] = (unsigned char)(character);
		}
		if (fclose(file) != 0)
			fputs("Failed to close config file stream.\n", stderr);
	}

	Configuration::configSyncAudio   = (azicfg[0] != 0x00) ? true : false;
	Configuration::configForceSync   = (azicfg[1] != 0x00) ? true : false;
	Configuration::configAIEmulation = (azicfg[2] != 0x00) ? true : false;
	Configuration::configVolume      = (azicfg[3] > 100) ? 100 : azicfg[3];

	snd->AI_Startup();
	bLockAddrRegister = false;
	if (*(u64*)(AudioInfo.HEADER + 0x10) == 0x117daa80bbc99d32 &&
		*(u8*)(AudioInfo.HEADER + 0x3D) == 0x45)
		bLockAddrRegister = true;
	if (*(u64*)(AudioInfo.HEADER + 0x10) == 0xB14B3F18E688A5B8 &&
		*(u8*)(AudioInfo.HEADER + 0x3D) == 0x50)
		bLockAddrRegister = true;
	if (*(u64*)(AudioInfo.HEADER + 0x10) == 0xEB7584E8519EA4E1 &&
		*(u8*)(AudioInfo.HEADER + 0x3D) == 0x4A)
		bLockAddrRegister = true;
	LockAddrRegisterValue = 0;
	return TRUE;
}

EXPORT void CALL CloseDLL(void) {
	dprintf("Call: CloseDLL()\n");
	if (snd != NULL)
	{
		snd->AI_Shutdown();
		delete snd;
		snd = NULL;
	}
}

EXPORT void CALL GetDllInfo(PLUGIN_INFO * PluginInfo) {
	PluginInfo->MemoryBswaped = TRUE;
	PluginInfo->NormalMemory  = FALSE;
	safe_strcpy(PluginInfo->Name, 100, PLUGIN_VERSION);
	PluginInfo->Type = PLUGIN_TYPE_AUDIO;
	PluginInfo->Version = 0x0101; // Set this to retain backwards compatibility
}

EXPORT void CALL ProcessAList(void) {
	if (snd == NULL)
		return;
	HLEStart ();
}

EXPORT void CALL RomOpen(void) 
{
	dprintf("Call: RomOpen()\n");
	if (snd == NULL)
		return;
	Dacrate = 0; // Forces a revisit to initialize audio
	snd->AI_ResetAudio();
}

EXPORT void CALL RomClosed(void) 
{
	dprintf("Call: RomClosed()\n");
	if (snd == NULL)
		return;
	snd->StopAudio();
}

EXPORT void CALL AiDacrateChanged(int SystemType) {
	u32 Frequency, video_clock;

	dprintf("Call: AiDacrateChanged()\n");
	if (snd == NULL)
		return;
	if (Dacrate == *AudioInfo.AI_DACRATE_REG)
		return;

	Dacrate = *AudioInfo.AI_DACRATE_REG & 0x00003FFF;
#ifdef _DEBUG
	if (Dacrate != *AudioInfo.AI_DACRATE_REG)
		MessageBoxA(
			NULL,
			"Unknown/reserved bits in AI_DACRATE_REG set.",
			"Warning",
			MB_ICONWARNING
		);
#endif
	switch (SystemType) {
		default         :  assert(FALSE);
		case SYSTEM_NTSC:  video_clock = 48681812; break;
		case SYSTEM_PAL :  video_clock = 49656530; break;
		case SYSTEM_MPAL:  video_clock = 48628316; break;
	}
	Frequency = video_clock / (Dacrate + 1);
	snd->AI_SetFrequency(Frequency);
}

EXPORT void CALL AiLenChanged(void) 
{
	if (snd == NULL)
		return;
	if (bLockAddrRegister == true)
	{
		if (LockAddrRegisterValue == 0)
			LockAddrRegisterValue = *AudioInfo.AI_DRAM_ADDR_REG;
		snd->AI_LenChanged(
			(AudioInfo.RDRAM + (LockAddrRegisterValue & 0x00FFFFF8)),
			*AudioInfo.AI_LEN_REG & 0x3FFF8);
	}
	else
	{
		snd->AI_LenChanged(
			(AudioInfo.RDRAM + (*AudioInfo.AI_DRAM_ADDR_REG & 0x00FFFFF8)),
			*AudioInfo.AI_LEN_REG & 0x3FFF8);
	}
}

EXPORT u32 CALL AiReadLength(void) {
	if (snd == NULL)
		return 0;
	*AudioInfo.AI_LEN_REG = snd->AI_ReadLength();
	return *AudioInfo.AI_LEN_REG;
}

EXPORT void CALL AiUpdate(Boolean Wait) {
	static int intCount = 0;

	if (snd == NULL)
	{
#if defined(_WIN32) || defined(_XBOX)
		Sleep(1);
#endif
		return;
	}
	snd->AI_Update(Wait);
	return;
}


#if defined(_WIN32) && !defined(_XBOX)
INT_PTR CALLBACK ConfigProc(
	HWND hDlg,  // handle to dialog box
	UINT uMsg,     // message
	WPARAM wParam, // first message parameter
	LPARAM lParam  // second message parameter
	) {
	UNREFERENCED_PARAMETER(lParam);
	int x;
	switch (uMsg) {
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_RESETCONTENT, 0, 0);
		for (x = 0; x < DSoundCnt; x++) {
			SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_ADDSTRING, 0, (long)DSoundDeviceName[x]);
		}
		SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_SETCURSEL, SelectedDSound, 0);
		SendMessage(GetDlgItem(hDlg, IDC_OLDSYNC), BM_SETCHECK, Configuration::configForceSync ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hDlg, IDC_AUDIOSYNC), BM_SETCHECK, Configuration::configSyncAudio ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hDlg, IDC_AI), BM_SETCHECK, Configuration::configAIEmulation ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETPOS, TRUE, Configuration::configVolume);
		SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETTICFREQ, 20, 0);
		SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETRANGEMIN, FALSE, 0);
		SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETRANGEMAX, FALSE, 100);
		if (Configuration::configVolume == 100)
		{
			SendMessage(GetDlgItem(hDlg, IDC_MUTE), BM_SETCHECK, BST_CHECKED, 0);
		}
		else
		{
			SendMessage(GetDlgItem(hDlg, IDC_MUTE), BM_SETCHECK, BST_UNCHECKED, 0);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			Configuration::configForceSync = SendMessage(GetDlgItem(hDlg, IDC_OLDSYNC), BM_GETSTATE, 0, 0) == BST_CHECKED ? true : false;
			Configuration::configSyncAudio = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSYNC), BM_GETSTATE, 0, 0) == BST_CHECKED ? true : false;
			Configuration::configAIEmulation = SendMessage(GetDlgItem(hDlg, IDC_AI), BM_GETSTATE, 0, 0) == BST_CHECKED ? true : false;
			SelectedDSound = (int)SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_GETCURSEL, 0, 0);
			safe_strcpy(Configuration::configDevice, 99, DSoundDeviceName[SelectedDSound]);
			Configuration::configVolume = SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_GETPOS, 0, 0);
			snd->SetVolume(Configuration::configVolume);

			FILE *file;
			file = fopen("Config/AziCfg.bin", "wb");
			if (file != NULL)
			{
				fprintf(file, "%c", Configuration::configSyncAudio);
				fprintf(file, "%c", Configuration::configForceSync);
				fprintf(file, "%c", Configuration::configAIEmulation);
				fprintf(file, "%c", Configuration::configVolume);
				fclose(file);
			}
			EndDialog(hDlg, 0);
			break;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		case IDC_MUTE:
			if (IsDlgButtonChecked(hDlg, IDC_MUTE))
			{
				SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETPOS, TRUE, 100);
				snd->SetVolume(100);
			}
			else {
				SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETPOS, TRUE, Configuration::configVolume);
				snd->SetVolume(Configuration::configVolume);
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
			int dwPosition = SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_GETPOS, 0, 0);
			if (dwPosition == 100)
			{
				SendMessage(GetDlgItem(hDlg, IDC_MUTE), BM_SETCHECK, BST_CHECKED, 0);
			}
			else
			{
				SendMessage(GetDlgItem(hDlg, IDC_MUTE), BM_SETCHECK, BST_UNCHECKED, 0);
			}
			Configuration::configVolume = dwPosition;
			snd->SetVolume(dwPosition);
		}
		break;
	}

	return FALSE;

}
#endif

// TODO: I think this can safely be removed
#ifdef _WIN32
BOOL CALLBACK DSEnumProc(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext)
{
	UNREFERENCED_PARAMETER(lpszDrvName);
	UNREFERENCED_PARAMETER(lpContext);
	//HWND hDlg = (HWND)lpContext;
	safe_strcpy(DSoundDeviceName[DSoundCnt], 99, lpszDesc);
	DSoundGUID[DSoundCnt] = lpGUID;
	if (strcmp(lpszDesc, Configuration::configDevice) == 0)
	{
		SelectedDSound = DSoundCnt;
	}
	DSoundCnt++;

	return TRUE;
}
#endif

int safe_strcpy(char* dst, size_t limit, const char* src)
{
#if defined(_MSC_VER) && !defined(_XBOX)
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

#ifdef USE_PRINTF
static const WORD MAX_CONSOLE_LINES = 500;
void RedirectIOToConsole() {
#if !defined(_XBOX)
	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;
	// allocate a console for this app
	FreeConsole();
	if (!AllocConsole())
		return;
	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);
	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "r");
	*stdin = *fp;
	setvbuf(stdin, NULL, _IONBF, 0);
	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	*stderr = *fp;
	setvbuf(stderr, NULL, _IONBF, 0);
	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog 
	// point to console as well
	ios::sync_with_stdio();
#endif
}

#endif