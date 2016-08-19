#include "stdafx.h"
#include "Config.h"

namespace Config
{


    static float getInitialDPIScaleX()
    {
        static float DPIScaleX = 0;
        if (!DPIScaleX) {
            HDC hdc = GetDC(NULL);
            DPIScaleX = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
            ReleaseDC(NULL, hdc);
        }
        return DPIScaleX;
    }
    static float getInitialDPIScaleY() {
        static float DPIScaleY = 0;
        if (!DPIScaleY) {
            HDC hdc = GetDC(NULL);
            DPIScaleY = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;
            ReleaseDC(NULL, hdc);
        }
        return DPIScaleY;
    }

    // Calculate initial X so window is in lower corner of any resolution screen
    int getDefaultKeyWindowX()
    {
        return int(DEFAULT_KEYWINDOW_LEFT_DIP * getInitialDPIScaleX());
    }

    // Calculate initial Y so window is in lower corner of any resolution screen
    int getDefaultKeyWindowY()
    {
        const int sy = GetSystemMetrics(SM_CYSCREEN);

        return sy - int((DEFAULT_KEYWINDOW_HEIGHT_DIP + DEFAULT_KEYWINDOW_BOTTOM_DIP) * getInitialDPIScaleY());
    }

    // Calculate height so window is approximately same size on any DPI screen
    int getDefaultKeyWindowWidth()
    {
        return int(DEFAULT_KEYWINDOW_WIDTH_DIP * getInitialDPIScaleX());
    }

    int getDefaultKeyWindowHeight()
    {
        return int(DEFAULT_KEYWINDOW_HEIGHT_DIP * getInitialDPIScaleY());
    }

    int getDefaultMouseWindowWidth()
    {
        return int(DEFAULT_MOUSEWINDOW_WIDTH_DIP * getInitialDPIScaleX());
    }
    int getDefaultMouseWindowHeight()
    {
        return int(DEFAULT_MOUSEWINDOW_HEIGHT_DIP * getInitialDPIScaleY());
    }



}