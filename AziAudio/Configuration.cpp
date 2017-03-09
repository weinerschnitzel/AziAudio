#include "Configuration.h"
#include "common.h"
#include <Windows.h>
#include <stdio.h>
#include "resource.h"
#include "SoundDriverInterface.h"

extern HINSTANCE hInstance; // DLL's HINSTANCE
extern SoundDriverInterface *snd;

bool Configuration::configAIEmulation;
bool Configuration::configSyncAudio;
bool Configuration::configForceSync;
unsigned long Configuration::configVolume;
char Configuration::configAudioLogFolder[500];
char Configuration::configDevice[100];

static char DSoundDeviceName[10][100];
static int DSoundCnt;
static int SelectedDSound;


// Dialog Procedures
#if defined(_WIN32)
INT_PTR CALLBACK ConfigProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

void Configuration::LoadDefaults()
{
	DSoundCnt = 0;
	SelectedDSound = 0;

	safe_strcpy(Configuration::configAudioLogFolder, 499, "D:\\");
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

	Configuration::configSyncAudio = (azicfg[0] != 0x00) ? true : false;
	Configuration::configForceSync = (azicfg[1] != 0x00) ? true : false;
	Configuration::configAIEmulation = (azicfg[2] != 0x00) ? true : false;
	Configuration::configVolume = (azicfg[3] > 100) ? 100 : azicfg[3];
}
#ifdef _WIN32
void Configuration::ConfigDialog(HWND hParent)
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONFIG), hParent, ConfigProc);
}

#define ABOUTMESSAGE \
	PLUGIN_VERSION\
	"\nby Azimer\n"\
	"\nHome: https://www.apollo64.com/\n"\
	"Source: https://github.com/Azimer/AziAudio/\n"\
	"\n"\
	"MusyX code credited to Bobby Smiles and Mupen64Plus\n"

void Configuration::AboutDialog(HWND hParent)
{
	MessageBoxA(hParent, ABOUTMESSAGE, "About", MB_OK|MB_ICONINFORMATION);
}
#endif

#if 0
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
#endif

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

#if 0
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
#endif