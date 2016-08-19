#include "stdafx.h"
#include "MouseWindow.h"
#include "LookKey.h"

// We need this static pointer for the keyboard hook callback.
// (This class is instantiated only once).
MouseWindow * MouseWindow::s_pThis;

MouseWindow::MouseWindow(GraphicsManager * graphicsManager, ConfigManager * config) :
    LayeredWindow(L"mousewindow", graphicsManager, config)
{

    // Make the window fullscreen
    init(GetSystemMetrics(SM_XVIRTUALSCREEN),
        GetSystemMetrics(SM_YVIRTUALSCREEN),
        GetSystemMetrics(SM_CXVIRTUALSCREEN),
        GetSystemMetrics(SM_CXVIRTUALSCREEN));

    initInputHook();
}

void MouseWindow::updatePos()
{

    throw_if_fail(
        SetWindowPos(m_hwnd, NULL, m_pos.x, m_pos.y, m_size.cx, m_size.cy, SWP_NOZORDER)
    );
    m_dirty = true;
}

LRESULT MouseWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DISPLAYCHANGE:
    {
        // Stay fullscreen
        m_pos = { GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN) };
        m_size = { GetSystemMetrics(SM_CXVIRTUALSCREEN),  GetSystemMetrics(SM_CXVIRTUALSCREEN) };
        m_dirty = true;
        break;
    }
    default:
        return LayeredWindow::HandleMessage(message, wParam, lParam);
    }
    return 0;
}

MouseWindow::~MouseWindow()
{
    unhookInputHook();
}

LRESULT CALLBACK MouseWindow::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        const MSLLHOOKSTRUCT  *info = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
        const UINT msg = UINT(wParam);
        POINT direction {}; // Used only for mousewheel messages

        if (msg == WM_MOUSEWHEEL)
        {
            const int16_t wheelDelta = HIWORD(info->mouseData);
            // Produce direction where {1,1} = right, down
            // Wheel delta is the opposite -  positive for rotation forwards (up)
            const int sign = (wheelDelta > 0) - (wheelDelta < 0);
            direction = { 0, -sign };
        }

        s_pThis->handleMouseButton(info->pt, direction, msg);
    }
    return CallNextHookEx(0, nCode, wParam, lParam);
}

void MouseWindow::initInputHook()
{
    // Set the static instance pointer
    // Needed for the LowLevelKeyboardProc callback to call instance methods
    s_pThis = this;

    // Install the hook
    m_inputHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
    throw_if_fail(m_inputHook != 0);
}

void MouseWindow::unhookInputHook()
{
    if (m_inputHook)
    {
        UnhookWindowsHookEx(m_inputHook);
        m_inputHook = 0;
    }
}

void MouseWindow::setEnabled(bool enabled)
{
    LayeredWindow::setEnabled(enabled);
    m_pConfigManager->SetValue(Config::MouseEnabled, enabled);
}

void MouseWindow::handleMouseButton(POINT pt, POINT direction, UINT msg)
{
    ScreenToClient(m_hwnd, &pt);
    auto & v = m_mouseItems;

    switch (msg)
    {
    case WM_LBUTTONDOWN:
    {
        v.emplace_back(new MouseLButtonItem(m_pGraphicsManager));
        v.back()->startDown(pt);
        break;
    }
    case WM_RBUTTONDOWN:
    {
        v.emplace_back(new MouseRButtonItem(m_pGraphicsManager));
        v.back()->startDown(pt);
        break;
    }
    case WM_MBUTTONDOWN:
    {
        v.emplace_back(new MouseMButtonItem(m_pGraphicsManager));
        v.back()->startDown(pt);
        break;
    }
    case WM_MOUSEWHEEL:
    {
        v.emplace_back(new MouseWheelItem(m_pGraphicsManager, direction));
        v.back()->startDown(pt);
        // Skip Down animation and immediately start outward ripple
        v.back()->startUp(pt); 
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    {
        // Find the item for this button already down
        auto found = std::find_if(v.begin(), v.end(),
            [msg](const std::unique_ptr<MouseItem> & k)
        {
            return (k->isActive() && k->getVkCode() == msg);
        });

        if (found != v.end()) {
            (*found)->startUp(pt);
        }
        break;
    }
    }
}


// Draw assuming beginDraw() already called.
void MouseWindow::drawClientArea()
{
    m_pRenderTarget->Clear(D2D1::ColorF(0, 0, 0, 0.0f));

    // Draw the handles
    const D2D1_POINT_2F pt = { float(m_mousePos.x), float(m_mousePos.y) };
    D2D1_ELLIPSE circle;

    if (m_LButtonDown)
    {
        circle = D2D1::Ellipse(pt + CLICK_HANDLE_OFFSET_L, float(CLICK_HANDLE_RADIUS), float(CLICK_HANDLE_RADIUS));
        m_pBrush->SetColor(CLICK_COLOR_L);
        m_pBrush->SetOpacity(CLICK_HANDLE_OPACITY);
        m_pRenderTarget->FillEllipse(circle, m_pBrush);
    }
    if (m_RButtonDown)
    {
        circle = D2D1::Ellipse(pt + CLICK_HANDLE_OFFSET_R, float(CLICK_HANDLE_RADIUS), float(CLICK_HANDLE_RADIUS));
        m_pBrush->SetColor(CLICK_COLOR_R);
        m_pBrush->SetOpacity(CLICK_HANDLE_OPACITY);
        m_pRenderTarget->FillEllipse(circle, m_pBrush);
    }
    if (m_MButtonDown)
    {
        circle = D2D1::Ellipse(pt + CLICK_HANDLE_OFFSET_M, float(CLICK_HANDLE_RADIUS), float(CLICK_HANDLE_RADIUS));
        m_pBrush->SetColor(CLICK_COLOR_M);
        m_pBrush->SetOpacity(CLICK_HANDLE_OPACITY);
        m_pRenderTarget->FillEllipse(circle, m_pBrush);
    }

    // Draw the content
    std::for_each(m_mouseItems.begin(), m_mouseItems.end(),
        std::bind(&MouseItem::draw, std::placeholders::_1,
            m_pRenderTarget,
            m_pItemLayer,
            m_pBrush,
            m_pLinearGradientBrush));

}

// Cleanup done periodically, not at any particular time
// (currently LayeredWindow::tickFrame() calls this)
void MouseWindow::periodicUpdate()
{
    auto & v = m_mouseItems;

    if (!v.empty())
    {
        // Update mouse position
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(m_hwnd, &pt);
        if (m_mousePos.x != pt.x || m_mousePos.y != pt.y)
        {
            m_mousePos = pt;
            m_dirty = true;
        }


        // Remove expired items from the list
        auto r = std::remove_if(v.begin(), v.end(), std::bind(&MouseItem::isDone, std::placeholders::_1));
        v.erase(r, v.end());

        // Fix "stuck" items (e.g. moved into admin window where we can't catch buttonup)
        short state = GetAsyncKeyState(VK_LBUTTON);
        m_LButtonDown = (state & 0x8000) != 0;
        state = GetAsyncKeyState(VK_RBUTTON);
        m_RButtonDown = (state & 0x8000) != 0;
        state = GetAsyncKeyState(VK_MBUTTON);
        m_MButtonDown = (state & 0x8000) != 0;

        auto found = std::for_each(v.begin(), v.end(),
            [this](const auto & k)
        {
            if (k->isActive() && (
                (!m_LButtonDown && k->getVkCode() == VK_LBUTTON) ||
                (!m_RButtonDown && k->getVkCode() == VK_RBUTTON) ||
                (!m_MButtonDown && k->getVkCode() == VK_MBUTTON)
                ))
            {
                k->startUp(m_mousePos);
            }
        });

    }

}
