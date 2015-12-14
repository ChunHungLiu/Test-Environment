/*
 *
 * Copyright 2015 Mateusz Paluszkiewicz
 *
 */

#pragma once

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG

#include <tchar.h>

#if defined(TESTENV_EXPORT) // inside DLL
#   define TESTENV_API extern "C" __declspec(dllexport)
#else // outside DLL
#   define TESTENV_API extern "C" __declspec(dllimport)
#endif  // TESTENV_EXPORT

TESTENV_API BOOL WINAPI Startup(HINSTANCE, HWND);
TESTENV_API LPTSTR WINAPI GetName();
TESTENV_API BOOL WINAPI Render();
TESTENV_API VOID WINAPI MsgProc(HWND, UINT, WPARAM, LPARAM);
TESTENV_API VOID WINAPI Shutdown();
TESTENV_API VOID WINAPI Resize(LONG, LONG);

BOOL WINAPI ShowErrorMessage(UINT messageIcon, LPTSTR wndTitle, LRESULT result) {
	TCHAR buffer[256];
	_stprintf_s(buffer, _T("Error: 0x%p"), result);
	MessageBox(nullptr, buffer, wndTitle, messageIcon);

	return FALSE;
}