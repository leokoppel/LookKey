#pragma once

#include "ConfigManager.h"

namespace Config {

    // Desired default dimensions in DIP
    const float DEFAULT_KEYWINDOW_WIDTH_DIP = 600;
    const float DEFAULT_KEYWINDOW_HEIGHT_DIP = 80;
    const float DEFAULT_KEYWINDOW_LEFT_DIP = 30;
    const float DEFAULT_KEYWINDOW_BOTTOM_DIP = 60;

    const float DEFAULT_MOUSEWINDOW_WIDTH_DIP = 100;
    const float DEFAULT_MOUSEWINDOW_HEIGHT_DIP = 100;

    // Functions to set up Config defaults with pixel values, based on DIP values
    int getDefaultKeyWindowX();
    int getDefaultKeyWindowY();
    int getDefaultKeyWindowWidth();
    int getDefaultKeyWindowHeight();
    int getDefaultMouseWindowWidth();
    int getDefaultMouseWindowHeight();
   

    // Ini sections, keys and default values
    CONFIG_KEY_INT KeyWindowWidth = { L"KeyDisplay", L"width", getDefaultKeyWindowWidth() };
    CONFIG_KEY_INT KeyWindowHeight = { L"KeyDisplay", L"height",  getDefaultKeyWindowHeight() };
    CONFIG_KEY_INT KeyWindowX = { L"KeyDisplay", L"x",   getDefaultKeyWindowX() };
    CONFIG_KEY_INT KeyWindowY = { L"KeyDisplay", L"y",  getDefaultKeyWindowY() };

    CONFIG_KEY_BOOL KeyEnabled = { L"KeyDisplay", L"enabled", true };
    CONFIG_KEY_BOOL MouseEnabled = { L"MouseDisplay", L"enabled", false };







}
