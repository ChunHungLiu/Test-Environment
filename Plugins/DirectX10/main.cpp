/*
 *
 * Copyright 2015 Mateusz Paluszkiewicz
 *
 */

#include <Windows.h>

#include <d3d10.h>
#include <atlbase.h>

#include "..\..\TestEnvironment\IPlugin.h"

CComPtr<ID3D10Device> m_pD3D10Device;
CComPtr<IDXGISwapChain> m_pDXGISwapChain;
ID3D10RenderTargetView* m_pD3D10RenderTargetView;
DXGI_SWAP_CHAIN_DESC m_swapChainDesc;
DXGI_MODE_DESC m_bufferDesc;
FLOAT m_clearColor[4] = { 0.f, 0.f, 1.f, 1.f };

TESTENV_API BOOL WINAPI Startup(HINSTANCE hInstance, HWND hWnd) {
	ZeroMemory(&m_swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	ZeroMemory(&m_bufferDesc, sizeof(DXGI_MODE_DESC));

	RECT wndRect;
	GetWindowRect(hWnd, &wndRect);

	m_bufferDesc.Width = wndRect.right - wndRect.left;
	m_bufferDesc.Height = wndRect.bottom - wndRect.top;
	m_bufferDesc.RefreshRate.Numerator = 60;
	m_bufferDesc.RefreshRate.Denominator = 1;
	m_bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	m_swapChainDesc.BufferCount = 1;
	m_swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	m_swapChainDesc.OutputWindow = hWnd;
	m_swapChainDesc.SampleDesc.Count = 1;
	m_swapChainDesc.SampleDesc.Quality = 0;
	m_swapChainDesc.Windowed = TRUE;

	m_swapChainDesc.BufferDesc = m_bufferDesc;

	HRESULT lastHR = D3D10CreateDeviceAndSwapChain(nullptr, D3D10_DRIVER_TYPE_HARDWARE, nullptr, NULL, D3D10_SDK_VERSION, &m_swapChainDesc, &m_pDXGISwapChain, &m_pD3D10Device);

	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("D3D10CreateDeviceAndSwapChain"), lastHR);

	ID3D10Texture2D *pBackBuffer;

	lastHR = m_pDXGISwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("IDXGISwapChain - GetBuffer"), lastHR);

	lastHR = m_pD3D10Device->CreateRenderTargetView(pBackBuffer, NULL, &m_pD3D10RenderTargetView);
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("ID3D10Device - CreateRenderTargetView"), lastHR);

	pBackBuffer->Release();

	m_pD3D10Device->OMSetRenderTargets(1, &m_pD3D10RenderTargetView, 0);

	D3D10_VIEWPORT vp = { 0, 0, m_swapChainDesc.BufferDesc.Width, m_swapChainDesc.BufferDesc.Height, 0, 1 };

	m_pD3D10Device->RSSetViewports(1, &vp);

	return TRUE;
}

TESTENV_API LPTSTR WINAPI GetName() {
	return TEXT("DirectX 10");
}

TESTENV_API BOOL WINAPI Render() {
	m_pD3D10Device->ClearRenderTargetView(m_pD3D10RenderTargetView, m_clearColor);

	HRESULT lastHR = m_pDXGISwapChain->Present(NULL, NULL);
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("IDXGISwapChain - Present"), lastHR);

	return TRUE;
}

TESTENV_API VOID WINAPI MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
}

TESTENV_API VOID WINAPI Shutdown() {
	m_pD3D10RenderTargetView->Release();
	m_pDXGISwapChain.Release();
	m_pD3D10Device.Release();

	m_pD3D10RenderTargetView = nullptr;
}

TESTENV_API VOID WINAPI Resize(LONG width, LONG height) {
	m_pD3D10Device->OMSetRenderTargets(0, 0, 0);

	m_pD3D10RenderTargetView->Release();

	HRESULT lastHR = m_pDXGISwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	ID3D10Texture2D* pBuffer;
	m_pDXGISwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&pBuffer);

	m_pD3D10Device->CreateRenderTargetView(pBuffer, NULL, &m_pD3D10RenderTargetView);

	pBuffer->Release();

	m_pD3D10Device->OMSetRenderTargets(1, &m_pD3D10RenderTargetView, NULL);

	D3D10_VIEWPORT vp;
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_pD3D10Device->RSSetViewports(1, &vp);
}