// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <shellapi.h> //Shell_NotifyIcon

#include <commctrl.h> //LoadIconMetric
#pragma comment(lib, "comctl32")

#include <D2D1.h> //Direct2D
#pragma comment(lib, "d2d1")

#include <dwrite.h> //DirectWrite
#pragma comment(lib, "dwrite")

#include <wincodec.h> // WIC
#include <UIAnimation.h> //IUIAnimationManager et al
#include <windowsx.h> // GET_X_LPARAM
#include <Commdlg.h> // ChooseColor


// C RunTime Header Files

#include <strsafe.h> //StringCchCopy
#include <objbase.h> //CoCreateInstance
#include <vector> //std::vector
#include <algorithm> //std::find and std::remove
#include <functional> //std::mem_fun_ref
#include <memory> // unique_ptr

#include <atlbase.h> // CComPtr
#include <comdef.h> // _com_error

#include <assert.h>

