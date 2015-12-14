/*
 *
 * Copyright 2015 Mateusz Paluszkiewicz
 *
 */

#include "stdafx.h"
#include "resource.h"

#include "IPlugin.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#if defined(_UNICODE)
using string = std::wstring;
#else
using string = std::string;
#endif

static TCHAR g_szWindowClass[] = TEXT("UniSuite");
static TCHAR g_szWindowTitle[] = TEXT("Test Environment - Engine: %s - Made by TheAifam5");

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

std::vector<string> GetFiles(LPCTSTR folder);
string GetExePath();

#if defined(_WIN64)
#define RenderPluginDLL "Render"
#define ResizePluginDLL "Resize"
#define GetNamePluginDLL "GetName"
#define MsgProcPluginDLL "MsgProc"
#define ShutdownPluginDLL "Shutdown"
#define StartupPluginDLL "Startup"
#elif defined(_WIN32)
#define RenderPluginDLL "_Render@0"
#define ResizePluginDLL "_Resize@8"
#define GetNamePluginDLL "_GetName@0"
#define MsgProcPluginDLL "_MsgProc@16"
#define ShutdownPluginDLL "_Shutdown@0"
#define StartupPluginDLL "_Startup@8"
#endif

struct IPlugin {
	decltype(&GetName) name;
	HMODULE module;
	BOOL active;

	inline bool operator==(const IPlugin& rhs) { return module == rhs.module; }
	inline bool operator!=(const IPlugin& rhs) { return !(*this == rhs); }
};

static std::vector<IPlugin> g_hPlugins;
static IPlugin* g_hActivePlugin;

decltype(&Startup) StartupPlugin;
decltype(&Render) RenderPlugin;
decltype(&MsgProc) MsgProcPlugin;
decltype(&Shutdown) ShutdownPlugin;
decltype(&Resize) ResizePlugin;

INT WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, INT nCmdShow) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	auto listOfPlugins = GetFiles(GetExePath().c_str());
	for (auto file : listOfPlugins) {
		auto module = LoadLibrary(file.c_str());
		auto GetNamePlugin = reinterpret_cast<decltype(&GetName)>(GetProcAddress(module, GetNamePluginDLL));
		if (GetNamePlugin)
			g_hPlugins.push_back({ GetNamePlugin, module, FALSE });
		else
			FreeLibrary(module);
	}

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_SELECTENGINE), NULL, DlgProc);

	if (g_hActivePlugin == nullptr)
		return 0x0;

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	//wcex.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	wcex.hbrBackground = nullptr;
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = g_szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex))
		return 0x1;

	StartupPlugin = reinterpret_cast<decltype(&Startup)>(GetProcAddress(g_hActivePlugin->module, StartupPluginDLL));
	RenderPlugin = reinterpret_cast<decltype(&Render)>(GetProcAddress(g_hActivePlugin->module, RenderPluginDLL));
	MsgProcPlugin = reinterpret_cast<decltype(&MsgProc)>(GetProcAddress(g_hActivePlugin->module, MsgProcPluginDLL));
	ShutdownPlugin = reinterpret_cast<decltype(&Shutdown)>(GetProcAddress(g_hActivePlugin->module, ShutdownPluginDLL));
	ResizePlugin = reinterpret_cast<decltype(&Resize)>(GetProcAddress(g_hActivePlugin->module, ResizePluginDLL));

	auto hWnd = CreateWindow(g_szWindowClass, TEXT("Test Environment - Engine: Unknown - Made by TheAifam5"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
		return 0x2;

	if (!StartupPlugin(hInstance, hWnd))
		return 0x3;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	TCHAR buffer[256] = { 0 };
	_stprintf_s(buffer, g_szWindowTitle, g_hActivePlugin->name());

	SetWindowText(hWnd, buffer);

	MSG msg;

	g_hActivePlugin->active = true;

	for (;;) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (!RenderPlugin())
			break;
	}

	ShutdownPlugin();

	for (auto& plugins : g_hPlugins)
		FreeLibrary(plugins.module);

	DestroyWindow(hWnd);
	UnregisterClass(g_szWindowClass, hInstance);

	#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
	#endif

	return static_cast<INT>(msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (g_hActivePlugin && g_hActivePlugin->active)
		MsgProcPlugin(hWnd, uMsg, wParam, lParam);

	switch (uMsg) {
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_SIZE:
			if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED)
				ResizePlugin(LOWORD(lParam), HIWORD(lParam));
			break;

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {

		case WM_INITDIALOG:
		{
			auto hWndCombo = GetDlgItem(hWnd, IDC_COMBOENGINES);
			for (auto custom : g_hPlugins)
				SendMessage(hWndCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(custom.name()));
			break;
		}


		case WM_COMMAND:
			switch (wParam) {
				case IDC_BUTTONACCEPT:
				{
					TCHAR szSelectedEngine[256] = { 0 };
					GetDlgItemText(hWnd, IDC_COMBOENGINES, szSelectedEngine, 255);

					for (auto& plugin : g_hPlugins) {
						if (StrCmp(plugin.name(), szSelectedEngine) == 0) {
							g_hActivePlugin = &plugin;
							break;
						}
					}

					EndDialog(hWnd, 0);
				}
				return TRUE;
			}
			break;
	}

	return FALSE;
}

string GetExePath() {
	TCHAR buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	string::size_type pos = string(buffer).find_last_of(TEXT("\\/"));
	return string(buffer).substr(0, pos);
}

std::vector<string> GetFiles(LPCTSTR folder) {
	std::vector<string> names;
	TCHAR buffer[MAX_PATH];
	wsprintf(buffer, TEXT("%s\\Plugins\\*.dll"), folder);
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(buffer, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				wsprintf(buffer, TEXT("%s\\Plugins\\%s"), folder, fd.cFileName);
				names.push_back(buffer);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}