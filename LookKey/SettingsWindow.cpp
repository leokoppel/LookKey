#include "stdafx.h"
#include "SettingsWindow.h"

#include "LookKey.h"
#include "Resource.h"

SettingsWindow::SettingsWindow(KeyWindow * keywin, MouseWindow * mousewin,
    GraphicsManager * graphics, ConfigManager * config) :
    BaseWindow(L"SettingsWindow"),
    m_pGraphicsManager(graphics),
    m_pConfigManager(config),
    m_pKeyWin(keywin),
    m_pMouseWin(mousewin)
{
    // Show / hide windows based on ini settings
    m_pKeyWin->setEnabled(m_pConfigManager->GetValue(Config::KeyEnabled));
    m_pMouseWin->setEnabled(m_pConfigManager->GetValue(Config::MouseEnabled));


    // Prepare WNDCLASSEX info (some is filled in during Create() call)
    WNDCLASSEX wcex = {};
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_KEYDISPLAY));
    wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
    wcex.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_KEYDISPLAY));

    // Get size of work area above taskbar (virtual screen coordinates)
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, NULL, &workArea, NULL);

    // Register the window class and create the window
    throw_if_fail(
        Create(L"LookKey",
            WS_POPUPWINDOW | WS_BORDER,
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
            wcex,
            workArea.right - SETTINGSWINDOW_WIDTH,
            workArea.bottom - SETTINGSWINDOW_HEIGHT,
            SETTINGSWINDOW_WIDTH,
            SETTINGSWINDOW_HEIGHT)
    );

    m_hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_TRAYMENU));
    InitTrayIcon();

    // TODO
    setEnabled(true);


}

SettingsWindow::~SettingsWindow()
{
    DestroyMenu(m_hMenu);
}


void SettingsWindow::InitTrayIcon()
{
    m_tray.cbSize = sizeof(m_tray);
    m_tray.uVersion = NOTIFYICON_VERSION_4;
    m_tray.hWnd = m_hwnd;
    m_tray.uCallbackMessage = WMAPP_NOTIFYCALLBACK;

    // Show title as the icon's tooltip.
    throw_if_fail(
        LoadStringW(GetModuleHandle(NULL), IDS_APP_TITLE, m_tray.szTip, ARRAYSIZE(m_tray.szTip))
    );

    // Load the icon for high DPI.
    throw_if_fail(
        LoadIconMetric(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_KEYDISPLAY), LIM_SMALL, &(m_tray.hIcon))
    );

    m_tray.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP;

    // Show the notification.
    throw_if_fail(
        Shell_NotifyIcon(NIM_ADD, &m_tray)
    );
    throw_if_fail(
        Shell_NotifyIcon(NIM_SETVERSION, &m_tray)
    );
}

void SettingsWindow::DeleteTrayIcon()
{
    throw_if_fail(
        Shell_NotifyIcon(NIM_DELETE, &m_tray)
    );
    DestroyIcon(m_tray.hIcon);
}

void SettingsWindow::ShowTrayContextMenu(POINT pt)
{

    if (m_hMenu)
    {
        // The context menu is really a "submenu" of the resource
        HMENU hSubMenu = GetSubMenu(m_hMenu, 0);
        if (hSubMenu)
        {
            // Set default (bolded) entry
            SetMenuDefaultItem(hSubMenu, IDM_SETTINGS, FALSE);

            // Allow right clicks to select menu items
            UINT uFlags = TPM_RIGHTBUTTON;
            // Set alignment to match system config
            if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0) {
                uFlags |= TPM_RIGHTALIGN;
            }
            else {
                uFlags |= TPM_LEFTALIGN;
            }

            SetForegroundWindow(m_hwnd);
            TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, m_hwnd, NULL);

        }
    }

}

void SettingsWindow::CreateGUI()
{
    WCHAR buf[255];
    HICON hIcon;
    RECT rect;
    GetClientRect(m_hwnd, &rect);

    // Logo / icon button
    LoadString(GetModuleHandle(NULL), IDS_APP_TITLE, buf, ARRAYSIZE(buf));
    HWND hIconButton = CreateWindow(L"BUTTON",
        buf,
        WS_VISIBLE | WS_CHILD | BS_ICON | BS_FLAT,
        10, 10,
        32, 32,
        m_hwnd,
        (HMENU)0,
        GetModuleHandle(NULL), NULL);
    LoadIconWithScaleDown(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_KEYDISPLAY), 32, 32, &hIcon);
    SendMessage(hIconButton, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    // Settings checkboxes
    LoadString(GetModuleHandle(NULL), IDS_SHOW_KEYS, buf, ARRAYSIZE(buf));
    CreateWindow(L"BUTTON",
        buf,
        WS_GROUP | WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        10, 45,
        200, 30,
        m_hwnd,
        (HMENU)IDC_SHOW_KEYS,
        GetModuleHandle(NULL), NULL);

    LoadString(GetModuleHandle(NULL), IDS_SHOW_MOUSE, buf, ARRAYSIZE(buf));
    CreateWindow(L"BUTTON",
        buf,
        WS_GROUP | WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        10, 75,
        200, 30,
        m_hwnd,
        (HMENU)IDC_SHOW_MOUSE,
        GetModuleHandle(NULL), NULL);

    // Exit button
    LoadString(GetModuleHandle(NULL), IDS_EXIT, buf, ARRAYSIZE(buf));
    CreateWindow(L"BUTTON",
        buf,
        WS_GROUP | WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rect.right - 100 - 10, rect.bottom - 40 - 10,
        100, 40,
        m_hwnd,
        (HMENU)IDM_EXIT,
        GetModuleHandle(NULL), NULL);

    // Set checkbox state based on window state
    if (m_pKeyWin->isEnabled())
    {
        CheckDlgButton(m_hwnd, IDC_SHOW_KEYS, BST_CHECKED);
    }
    if (m_pMouseWin->isEnabled())
    {
        CheckDlgButton(m_hwnd, IDC_SHOW_MOUSE, BST_CHECKED);
    }
}

void SettingsWindow::setEnabled(bool enabled)
{
    if (enabled) {
        ShowWindow(m_hwnd, SW_SHOW);
    }
    else {
        ShowWindow(m_hwnd, SW_HIDE);
    }

    m_pKeyWin->setEditMode(enabled);
}

LRESULT SettingsWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    static UINT TASKBAR_CREATED = 0;

    switch (message)
    {
    case WM_CREATE:
    {
        TASKBAR_CREATED = RegisterWindowMessage(L"TaskbarCreated");

        CreateGUI();

        break;
    }
    case WM_CLOSE:
    {
        // We want to minimize on clicking the "X"
        CloseWindow(m_hwnd); //  misnomer, it does minimize
        break;
    }
    case WM_ACTIVATEAPP:
    {
        // Hide the settings if another application's window is activated
        if (wParam == FALSE) {
            setEnabled(false);
        }
        break;
    }
    case WM_ACTIVATE:
    {
        const bool active = LOWORD(wParam) != WA_INACTIVE;

        if (active) {
            // Keep mouse window on top
            SetWindowPos(m_hwnd, m_pMouseWin->hWnd(), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        break;
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(m_hwnd);
            break;
        case IDM_SETTINGS:
            setEnabled(true);
            break;
        case IDC_SHOW_KEYS:
            m_pKeyWin->setEnabled(IsDlgButtonChecked(m_hwnd, IDC_SHOW_KEYS) == BST_CHECKED);
            break;
        case IDC_SHOW_MOUSE:
            m_pMouseWin->setEnabled(IsDlgButtonChecked(m_hwnd, IDC_SHOW_MOUSE) == BST_CHECKED);
            break;
        default:
            return DefWindowProc(m_hwnd, message, wParam, lParam);
        } // end switch(wmId)
        break;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    {
        Debug::out << m_classname << "WM_CTLCOLORBTN" << std::endl;
        // Give checkbox text white background (or at least one matching the window)
        return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
    }
    case WM_PAINT:
    {
        OnPaint();
        return DefWindowProc(m_hwnd, message, wParam, lParam);
    }
    break;
    case WMAPP_NOTIFYCALLBACK:
        switch (LOWORD(lParam))
        {
        case NIN_SELECT:
        {
            // Show the window
            setEnabled(true);
            break;
        }
        case WM_CONTEXTMENU:
        {
            POINT const clickPoint = { LOWORD(wParam), HIWORD(wParam) };
            ShowTrayContextMenu(clickPoint);
            break;
        }
        case  WM_COMMAND:
            Debug::out << "WM_COMMAND" << std::endl;
            break;
        }
        break;
    case WM_DESTROY:
        DestroyWindow(m_pKeyWin->hWnd());
        DestroyWindow(m_pMouseWin->hWnd());
        DeleteTrayIcon();
        PostQuitMessage(0);
        break;
    case WM_WINDOWPOSCHANGED: {
        const WINDOWPOS * newPos = reinterpret_cast<WINDOWPOS*>(lParam);

        WCHAR buf[100] = L"nobody";
        if (newPos->hwndInsertAfter) {
            RealGetWindowClass(newPos->hwndInsertAfter, buf, ARRAYSIZE(buf));
        }
        Debug::out << m_classname << " WM_WINDOWPOSCHANGED: " << newPos->cx << " x " << newPos->cy
            << " at " << newPos->x << "," << newPos->y
            << " after " << buf << std::endl;

        break;
    }
    default:
        // Check for taskbar message, not known at compile-time
        if (message == TASKBAR_CREATED) {
            InitTrayIcon();
        }

        return DefWindowProc(m_hwnd, message, wParam, lParam);
    }
    return 0;
}

void SettingsWindow::OnPaint()
{
}

