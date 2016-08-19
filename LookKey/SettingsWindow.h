#pragma once

#include "KeyWindow.h"
#include "MouseWindow.h"

const int SETTINGSWINDOW_WIDTH = 400; //px
const int SETTINGSWINDOW_HEIGHT = 150;

class SettingsWindow : public BaseWindow<SettingsWindow>
{
public:
    SettingsWindow(KeyWindow * keywin, MouseWindow * mousewin,
        GraphicsManager * graphics, ConfigManager * config);
    ~SettingsWindow();

    void OnPaint();
    void setEnabled(bool enabled);
    LRESULT CALLBACK HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);


private:
    void InitTrayIcon();
    void DeleteTrayIcon();
    void ShowTrayContextMenu(POINT pt);
    void CreateGUI();

    // System tray icon data
    NOTIFYICONDATA m_tray = {};
    HMENU m_hMenu = 0;

    // Helper objects
    GraphicsManager * m_pGraphicsManager;
    ConfigManager * m_pConfigManager;

    // Layered window pointers
    KeyWindow * m_pKeyWin;
    MouseWindow * m_pMouseWin;

    // Graphics resources
    CComPtr<ID2D1RenderTarget> m_pRenderTarget;

};


const UINT WMAPP_NOTIFYCALLBACK = WM_APP + 1;


