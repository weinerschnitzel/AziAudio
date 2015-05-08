#ifndef _XBOX_DEPP_H__COMMON_
#define _XBOX_DEPP_H__COMMON_

#if _MSC_VER > 1000
#pragma once
#endif //_MSC_VER > 1000

#pragma warning(disable:4018)	// signed/unsigned mismatch
#pragma warning(disable:4101)	// unreferenced local variable
#pragma warning(disable:4244)	// conversion, possible loss of data
#pragma warning(disable:4731)	// frame pointer register modified by inline assembly code


#ifndef _XBOX_ICC
#include <xtl.h>
#else
#include "my_types.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#define XAUDIO_LIBRARIES_UNAVAILABLE

#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)

// Message Box arg's, unused on XBOX
#define MB_ABORTRETRYIGNORE 0x00000002L
#define MB_CANCELTRYCONTINUE 0x00000006L
#define MB_HELP 0x00004000L
#define MB_OK 0x00000000L
#define MB_OKCANCEL 0x00000001L
#define MB_RETRYCANCEL 0x00000005L
#define MB_YESNO 0x00000004L
#define MB_YESNOCANCEL 0x00000003L
#define MB_ICONASTERISK 0x00000040L
#define MB_ICONERROR 0x00000010L
#define MB_ICONEXCLAMATION 0x00000030L
#define MB_ICONHAND 0x00000010L
#define MB_ICONINFORMATION 0x00000040L
#define MB_ICONQUESTION 0x00000020L
#define MB_ICONSTOP 0x00000010L
#define MB_ICONWARNING 0x00000030L

// ShowWindow arg's. unused on XBOX
#define SW_HIDE			 0
#define SW_SHOW			 5

BOOL PathFileExists(const char *pszPath);
int MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode);
BOOL IsWindow(HWND hWnd);
BOOL ShowWindow(HWND hWnd, int CmdShow);
BOOL SetWindowText(HWND hWnd, LPCTSTR lpString);
LONG SetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong);
BOOL GetClientRect(HWND hWnd, LPRECT lpRect);
int ShowCursor(BOOL bShow);
int GetDlgCtrlID(HWND hWnd);
HWND GetDlgItem(HWND hDlg, int nIDDlgItem);
DWORD GetModuleFileName(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
BOOL StrTrim(LPSTR psz, LPCSTR pszTrimChars);
BOOL WaitMessage(void);

#if defined(__cplusplus)
}
#endif

#endif //_XBOX_DEPP_H__COMMON_
