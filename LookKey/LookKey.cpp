// LookKey.cpp : Defines the entry point for the application.

#include "stdafx.h"
#include "LookKey.h"

#include "Graphics.h"
#include "Config.h"
#include "KeyWindow.h"
#include "MouseWindow.h"
#include "SettingsWindow.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    MSG msg{};

    if (SUCCEEDED(CoInitialize(NULL)))
    {

        // Create graphics and animation objects
        GraphicsManager graphicsManager{};

        // Load config file
        ConfigManager config{};

        // Create and initialize windows
        MouseWindow mousewin(&graphicsManager, &config);
        KeyWindow keywin(&graphicsManager, &config);
        SettingsWindow mainwin(&keywin, &mousewin, &graphicsManager, &config);
    
        // Main message loop
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // All COM objects must have been destructed by this point
    CoUninitialize();

    return (int)msg.wParam;
}


