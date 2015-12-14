/*
 *
 * Copyright 2015 Mateusz Paluszkiewicz
 *
 */

#include <Windows.h>
#include <d3d9.h>
#include <atlbase.h>

#include "..\..\TestEnvironment\IPlugin.h"

CComPtr<IDirect3D9Ex> m_pDirect3D9Ex;
CComPtr<IDirect3DDevice9Ex> m_pDirect3DDevice9Ex;
D3DPRESENT_PARAMETERS m_d3dpp;
D3DCOLOR m_clearColor = D3DCOLOR_XRGB(0, 0, 255);

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved) {
	return TRUE;
}

TESTENV_API BOOL WINAPI Startup(HINSTANCE hInstance, HWND hWnd) {
	ZeroMemory(&m_d3dpp, sizeof(D3DPRESENT_PARAMETERS));

	HRESULT lastHR = Direct3DCreate9Ex(D3D_SDK_VERSION, &m_pDirect3D9Ex);

	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("Direct3DCreate9Ex"), lastHR);


	D3DDISPLAYMODE d3ddm = { 0 };

	lastHR = m_pDirect3D9Ex->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("IDirect3D9Ex - GetAdapterDisplayMode"), lastHR);

	D3DCAPS9 d3dcaps;

	lastHR = m_pDirect3D9Ex->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dcaps);
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("IDirect3D9Ex - GetDeviceCaps"), lastHR);

	DWORD behaviorFlags = 0;
	if (d3dcaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
		behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		if (d3dcaps.DevCaps & D3DDEVCAPS_PUREDEVICE) {
			behaviorFlags |= D3DCREATE_PUREDEVICE;
		}
	} else {
		behaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	D3DFORMAT adapterFormat = d3ddm.Format;

	if (SUCCEEDED(lastHR = m_pDirect3D9Ex->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8)))
		m_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	else if (SUCCEEDED(lastHR = m_pDirect3D9Ex->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8)))
		m_d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
	else if (SUCCEEDED(lastHR = m_pDirect3D9Ex->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16)))
		m_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	else
		return ShowErrorMessage(MB_ICONERROR, TEXT("Direct3DCreate9Ex"), lastHR);

	m_d3dpp.BackBufferFormat = adapterFormat;
	m_d3dpp.BackBufferCount = 1;
	m_d3dpp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
	m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_d3dpp.hDeviceWindow = hWnd;
	m_d3dpp.Windowed = TRUE;
	m_d3dpp.EnableAutoDepthStencil = TRUE;
	m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	RECT wndRect;
	GetWindowRect(hWnd, &wndRect);
	m_d3dpp.BackBufferWidth = wndRect.right - wndRect.left;
	m_d3dpp.BackBufferHeight = wndRect.bottom - wndRect.top;

	lastHR = m_pDirect3D9Ex->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, behaviorFlags, &m_d3dpp, NULL, &m_pDirect3DDevice9Ex);

	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("IDirect3DDevice9Ex - CreateDeviceEx"), lastHR);

	m_pDirect3DDevice9Ex->SetRenderState(D3DRS_LIGHTING, FALSE);    // turn off the 3D lighting
	m_pDirect3DDevice9Ex->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);    // turn off culling
	m_pDirect3DDevice9Ex->SetRenderState(D3DRS_ZENABLE, TRUE);    // turn on the z-buffer

	return SUCCEEDED(lastHR);
}

TESTENV_API LPTSTR WINAPI GetName() {
	return TEXT("DirectX 9");
}

TESTENV_API BOOL WINAPI Render() {
	HRESULT lastHR = m_pDirect3DDevice9Ex->Clear(NULL, nullptr, D3DCLEAR_TARGET, m_clearColor, 1.0f, NULL);
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("IDirect3DDevice9Ex - Clear - Target"), lastHR);

	lastHR = m_pDirect3DDevice9Ex->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("IDirect3DDevice9Ex - Clear - ZBuffer"), lastHR);

	lastHR = m_pDirect3DDevice9Ex->BeginScene();
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("IDirect3DDevice9Ex - BeginScene"), lastHR);


	lastHR = m_pDirect3DDevice9Ex->EndScene();
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("IDirect3DDevice9Ex - EndScene"), lastHR);

	lastHR = m_pDirect3DDevice9Ex->Present(nullptr, nullptr, nullptr, nullptr);
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("IDirect3DDevice9Ex - Present"), lastHR);

	return TRUE;
}

TESTENV_API VOID WINAPI MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
}

TESTENV_API VOID WINAPI Shutdown() {
	m_pDirect3DDevice9Ex.Release();
	m_pDirect3D9Ex.Release();
}

TESTENV_API VOID WINAPI Resize(LONG width, LONG height) {
	m_d3dpp.BackBufferWidth = width;
	m_d3dpp.BackBufferHeight = height;

	HRESULT lastHR = m_pDirect3DDevice9Ex->ResetEx(&m_d3dpp, nullptr);
	if (FAILED(lastHR))
		ShowErrorMessage(MB_ICONERROR, TEXT("IDirect3DDevice9Ex - ResetEx"), lastHR);
}