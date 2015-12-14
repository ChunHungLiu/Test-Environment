/*
 *
 * Copyright 2015 Mateusz Paluszkiewicz
 *
 */

#include <Windows.h>

#include <gl\gl.h>
#include <gl\glu.h>

#include "..\..\TestEnvironment\IPlugin.h"

HWND m_hWnd;
HDC	m_hDC;
HGLRC m_hRC;
GLuint m_PixelFormat;

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved) {
	return TRUE;
}

TESTENV_API BOOL WINAPI Startup(HINSTANCE hInstance, HWND hWnd) {
	m_hWnd = hWnd;
	m_hDC = GetDC(hWnd);
	if (!m_hDC)
		ShowErrorMessage(MB_ICONERROR, TEXT("GetDC"), GetLastError());

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		16,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	m_PixelFormat = ChoosePixelFormat(m_hDC, &pfd);
	if (!m_PixelFormat)
		return ShowErrorMessage(MB_ICONERROR, TEXT("ChoosePixelFormat"), GetLastError());

	if (!SetPixelFormat(m_hDC, m_PixelFormat, &pfd))
		return ShowErrorMessage(MB_ICONERROR, TEXT("SetPixelFormat"), GetLastError());

	m_hRC = wglCreateContext(m_hDC);
	if (!m_hRC)
		return ShowErrorMessage(MB_ICONERROR, TEXT("wglCreateContext"), GetLastError());

	if (!wglMakeCurrent(m_hDC, m_hRC))
		return ShowErrorMessage(MB_ICONERROR, TEXT("wglMakeCurrent"), GetLastError());

	SetForegroundWindow(hWnd);

	SetFocus(hWnd);

	RECT wndRect;
	GetWindowRect(hWnd, &wndRect);
	Resize(wndRect.right - wndRect.left, wndRect.bottom - wndRect.top);

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	return TRUE;
}

TESTENV_API LPTSTR WINAPI GetName() {
	return TEXT("OpenGL");
}

TESTENV_API VOID WINAPI MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_SIZE:
			Resize(LOWORD(lParam), HIWORD(lParam));
			break;
	}
}

TESTENV_API BOOL WINAPI Render() {
	glClearColor(0, 0, 1, 0);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();

	return SwapBuffers(m_hDC);
}

TESTENV_API VOID WINAPI Shutdown() {
	if (m_hRC) {
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_hRC);
	}

	if (m_hDC)
		ReleaseDC(m_hWnd, m_hDC);

	m_hRC = nullptr;
	m_hDC = nullptr;
}

TESTENV_API VOID WINAPI Resize(LONG width, LONG height) {
	glViewport(0, 0, width, height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matri
														// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();
}