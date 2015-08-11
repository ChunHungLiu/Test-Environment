/*
 *
 * Copyright 2015 Mateusz Paluszkiewicz
 *
 */

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <commctrl.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>

// DirectX Versions
#include <d3d9.h>

#include <d3d10_1.h>
#include <d3d10.h>
#include <d3dx10math.h>

#include <d3d11.h>
#include <d3d11_1.h>
#include <DirectXMath.h>

#include <gl\gl.h>
#include <gl\glu.h>

// STL for plugins
#include <vector>
#include <unordered_map>

float RandomFloat(float a, float b);