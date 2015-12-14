/*
 *
 * Copyright 2015 Mateusz Paluszkiewicz
 *
 */

#include <Windows.h>

#include <wrl\client.h>

#include <d3d12.h>
#include <DXGI1_4.h>

#include "..\..\TestEnvironment\IPlugin.h"

using namespace Microsoft::WRL;

ComPtr<ID3D12Device> m_dxDevice;
ComPtr<ID3D12CommandQueue> m_cmdQueue;
ComPtr<ID3D12CommandAllocator> m_cmdAllocator;
ComPtr<ID3D12GraphicsCommandList> m_cmdList;
ComPtr<IDXGISwapChain3> m_swapChain;
ComPtr<ID3D12Fence>  m_queueFence;
ComPtr<IDXGIFactory3> m_dxgiFactory;
ComPtr<ID3D12DescriptorHeap> m_descriptorHeapRTV;
ComPtr<ID3D12Resource> m_renderTarget[2];
D3D12_CPU_DESCRIPTOR_HANDLE m_handleRTV[2];
D3D12_VIEWPORT m_viewport;
HANDLE m_hFenceEvent;

FLOAT m_clearColor[4] = { 0, 1.0f, 1.0f, 0 };

VOID SetResourceBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
VOID WaitForCommandQueue(ID3D12CommandQueue* pCommandQueue);

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved) {
	return TRUE;
}

TESTENV_API BOOL WINAPI Startup(HINSTANCE hInstance, HWND hWnd) {
	HRESULT lastHR = CreateDXGIFactory2(NULL, IID_PPV_ARGS(m_dxgiFactory.GetAddressOf()));
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("CreateDXGIFactory2"), lastHR);

	ComPtr<IDXGIAdapter> adapter;

	lastHR = m_dxgiFactory->EnumAdapters(0, adapter.GetAddressOf());
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("DXGIAdapter - EnumAdapters"), lastHR);

	lastHR = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(m_dxDevice.GetAddressOf()));
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("D3D12CreateDevice"), lastHR);

	lastHR = m_dxDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_cmdAllocator.GetAddressOf()));
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("ID3D12Device - CreateCommandAllocator"), lastHR);

	D3D12_COMMAND_QUEUE_DESC descCommandQueue;
	ZeroMemory(&descCommandQueue, sizeof(descCommandQueue));
	descCommandQueue.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	descCommandQueue.Priority = 0;
	descCommandQueue.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	lastHR = m_dxDevice->CreateCommandQueue(&descCommandQueue, IID_PPV_ARGS(m_cmdQueue.GetAddressOf()));
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("ID3D12Device - CreateCommandQueue"), lastHR);

	m_hFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	lastHR = m_dxDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_queueFence.GetAddressOf()));
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("ID3D12Device - CreateFence"), lastHR);

	DXGI_SWAP_CHAIN_DESC descSwapChain;
	ZeroMemory(&descSwapChain, sizeof(descSwapChain));
	descSwapChain.BufferCount = 2;
	descSwapChain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	descSwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	descSwapChain.OutputWindow = hWnd;
	descSwapChain.SampleDesc.Count = 1;
	descSwapChain.Windowed = TRUE;
	descSwapChain.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	descSwapChain.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	lastHR = m_dxgiFactory->CreateSwapChain(m_cmdQueue.Get(), &descSwapChain, (IDXGISwapChain**)m_swapChain.GetAddressOf());
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("IDXGIFactory3 - CreateSwapChain"), lastHR);

	lastHR = m_dxDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAllocator.Get(), nullptr, IID_PPV_ARGS(m_cmdList.GetAddressOf()));
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("ID3D12Device - CreateCommandList"), lastHR);

	D3D12_DESCRIPTOR_HEAP_DESC descHeap;
	ZeroMemory(&descHeap, sizeof(descHeap));
	descHeap.NumDescriptors = 2;
	descHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	lastHR = m_dxDevice->CreateDescriptorHeap(&descHeap, IID_PPV_ARGS(m_descriptorHeapRTV.GetAddressOf()));
	if (FAILED(lastHR))
		return ShowErrorMessage(MB_ICONERROR, TEXT("ID3D12Device - CreateDescriptorHeap"), lastHR);

	UINT strideHandleBytes = m_dxDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (UINT i = 0; i < descSwapChain.BufferCount; ++i) {
		lastHR = m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_renderTarget[i].GetAddressOf()));
		if (FAILED(lastHR))
			return ShowErrorMessage(MB_ICONERROR, TEXT("IDXGISwapChain3 - GetBuffer"), lastHR);

		m_handleRTV[i] = m_descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart();
		m_handleRTV[i].ptr += i * strideHandleBytes;
		m_dxDevice->CreateRenderTargetView(m_renderTarget[i].Get(), nullptr, m_handleRTV[i]);
	}

	RECT wndRect;
	GetWindowRect(hWnd, &wndRect);
	Resize(wndRect.right - wndRect.left, wndRect.bottom - wndRect.top);

	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	m_viewport.MinDepth = 0;
	m_viewport.MaxDepth = 1;

	return TRUE;
}

TESTENV_API LPTSTR WINAPI GetName() {
	return TEXT("DirectX 12");
}

TESTENV_API BOOL WINAPI Render() {
	static int count = 0;
	int targetIndex = m_swapChain->GetCurrentBackBufferIndex();

	SetResourceBarrier(m_cmdList.Get(), m_renderTarget[targetIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_cmdList->RSSetViewports(1, &m_viewport);
	m_cmdList->ClearRenderTargetView(m_handleRTV[targetIndex], m_clearColor, 0, nullptr);

	SetResourceBarrier(m_cmdList.Get(), m_renderTarget[targetIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	m_cmdList->Close();

	ID3D12CommandList* pCommandList = m_cmdList.Get();
	m_cmdQueue->ExecuteCommandLists(1, &pCommandList);
	m_swapChain->Present(1, 0);

	WaitForCommandQueue(m_cmdQueue.Get());
	m_cmdAllocator->Reset();
	m_cmdList->Reset(m_cmdAllocator.Get(), nullptr);
	count++;

	return TRUE;
}

TESTENV_API VOID WINAPI MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
}

TESTENV_API VOID WINAPI Shutdown()
{
	CloseHandle(m_hFenceEvent);
	m_hFenceEvent = nullptr;
}

VOID SetResourceBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
	D3D12_RESOURCE_BARRIER descBarrier;
	ZeroMemory(&descBarrier, sizeof(descBarrier));
	descBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	descBarrier.Transition.pResource = resource;
	descBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	descBarrier.Transition.StateBefore = before;
	descBarrier.Transition.StateAfter = after;
	commandList->ResourceBarrier(1, &descBarrier);
}

VOID WaitForCommandQueue(ID3D12CommandQueue* pCommandQueue) {
	static UINT64 frames = 0;
	m_queueFence->SetEventOnCompletion(frames, m_hFenceEvent);
	pCommandQueue->Signal(m_queueFence.Get(), frames);
	WaitForSingleObject(m_hFenceEvent, INFINITE);
	frames++;
}

TESTENV_API VOID WINAPI Resize(LONG width, LONG height) {
	m_viewport.Width = (FLOAT)width;
	m_viewport.Height = (FLOAT)height;
}