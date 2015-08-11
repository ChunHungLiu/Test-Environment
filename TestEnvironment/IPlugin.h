/*
 *
 * Copyright 2015 Mateusz Paluszkiewicz
 *
 */

#pragma once

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