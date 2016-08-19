#include "stdafx.h"
#include "KeyWindow.h"

#include "LookKey.h"
#include "Config.h"
#include "KeyStrokeItem.h"

// We need this static pointer for the keyboard hook callback.
// (This class is instantiated only once).
KeyWindow * KeyWindow::s_pThis;

KeyWindow::KeyWindow(GraphicsManager * graphicsManager, ConfigManager * config) :
    LayeredWindow(L"keywindow", graphicsManager, config)
{
    init(config->GetValue(Config::KeyWindowX),
        config->GetValue(Config::KeyWindowY),
        config->GetValue(Config::KeyWindowWidth),
        config->GetValue(Config::KeyWindowHeight));

    initInputHook();

    // Create text for edit mode
    CComPtr<IDWriteFactory> pDWriteFactory;
    CComPtr<IDWriteTextFormat> pTextFormat;
    graphicsManager->getDWriteFactory(&pDWriteFactory);
    graphicsManager->getDWriteTextFormat(&pTextFormat);

    WCHAR buf[255];
    const UINT len = LoadString(GetModuleHandle(NULL), IDS_EDIT_INSTRUCTION, buf, ARRAYSIZE(buf));
    throw_if_fail(pDWriteFactory->CreateTextLayout(
        buf,
        len,
        pTextFormat,
        0,
        0,
        &m_pTextLayout
    ));
    m_pTextLayout->SetFontStyle(DWRITE_FONT_STYLE_ITALIC, { 0, len });

}

KeyWindow::~KeyWindow()
{
    unhookInputHook();
}

LRESULT KeyWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_NCHITTEST: {
        // Allow dragging by client area
        LRESULT hit = DefWindowProc(m_hwnd, message, wParam, lParam);
        if (hit == HTBORDER) assert(0);
        if (hit == HTCLIENT) {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(m_hwnd, &pt);
            return clientHitTest(pt.x, pt.y);
        }
        return hit;
        break;
    }
    case WM_SETCURSOR:
    {
        // Set the cursor when in the draggable client area
        // (Since we return HTCAPTION in WM_NCHITTEST, the class cursor is not shown)
        LRESULT hit = LOWORD(lParam);
        if (hit == HTCAPTION) {
            // "The LoadCursor function loads the cursor resource only if it has not been loaded;
            // otherwise, it retrieves the handle to the existing resource."
            // - so it's ok to load every time like this
            SetCursor(LoadCursor(nullptr, IDC_SIZEALL));

        }
        else {
            return DefWindowProc(m_hwnd, message, wParam, lParam);
        }
        break;
    }
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO * pMM = reinterpret_cast<MINMAXINFO*>(lParam);

        pMM->ptMinTrackSize = { CXMINTRACK, CYMINTRACK };
        if (CXMAXTRACK) {
            pMM->ptMaxTrackSize = { CXMAXTRACK, CYMAXTRACK };
        }

        return 0;
        break;
    }
    case WM_DESTROY:
    {
        // Save final window size and height
        m_pConfigManager->SetValue(Config::KeyWindowX, m_pos.x);
        m_pConfigManager->SetValue(Config::KeyWindowY, m_pos.y);
        m_pConfigManager->SetValue(Config::KeyWindowWidth, m_size.cx);
        m_pConfigManager->SetValue(Config::KeyWindowHeight, m_size.cy);
    }
    default:
        return LayeredWindow::HandleMessage(message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK KeyWindow::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT *info = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

        // Add extended bit to vkCode:
        BOOL extended = (info->flags & LLKHF_EXTENDED) != 0;
        UINT scanCode = info->scanCode | (extended << 8);

        switch (wParam) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            s_pThis->handleKeyDown(info->vkCode, scanCode);
            //Debug::out << std::hex << "DOWN vk 0x" << info->vkCode << " sc 0x" << scanCode;
            //Debug::out << std::endl;

            break;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
            s_pThis->handleKeyUp(info->vkCode, scanCode);
            break;
        default:
            Debug::out << "Unknown wparam " << wParam << std::endl;
        }


    }
    return CallNextHookEx(0, nCode, wParam, lParam);
}

void KeyWindow::initInputHook()
{
    // Set the static instance pointer
    // Needed for the LowLevelKeyboardProc callback to call instance methods
    s_pThis = this;

    // Install the hook
    HHOOK keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    throw_if_fail(keyboard_hook != 0);
}

void KeyWindow::unhookInputHook()
{
    if (m_inputHook)
    {
        UnhookWindowsHookEx(m_inputHook);
        m_inputHook = 0;
    }
}


void KeyWindow::setEnabled(bool enabled)
{
    LayeredWindow::setEnabled(enabled);
    m_pConfigManager->SetValue(Config::KeyEnabled, enabled);
}

void KeyWindow::setEditMode(bool v)
{
    if (v) {
        return enterEditMode();
    }
    else {
        return exitEditMode();
    }
}

void KeyWindow::toggleEditMode()
{
    return setEditMode(!inEditMode());
}


void KeyWindow::enterEditMode()
{
    m_editMode = 1;
    EditWindowLong(m_hwnd, GWL_EXSTYLE, WS_EX_TRANSPARENT, 0);
}

void KeyWindow::exitEditMode()
{
    m_editMode = 0;
    EditWindowLong(m_hwnd, GWL_EXSTYLE, WS_EX_TRANSPARENT, 1);
    m_dirty = true;
}

BOOL KeyWindow::inEditMode() const
{
    return m_editMode;
}

// Return scale to use in client area transform, based on window size
float KeyWindow::calculateScale() const {
    // Set transform based on size (use only y size for now)
    return float(m_size.cy) / Config::getDefaultKeyWindowHeight(); //TODO: what if dpi changes?
}

// Draw assuming beginDraw() already called.
void KeyWindow::drawClientArea()
{

    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);

    m_pRenderTarget->SetTransform(
        D2D1::Matrix3x2F::Scale(D2D1::SizeF(1, 1), D2D1::Point2F(0, 0)));
    m_pRenderTarget->Clear(D2D1::ColorF(0, 0, 0, 0.0f));


    const float B = static_cast<float>(BORDER_WIDTH);
    const float IB = static_cast<float>(BORDER_HITTEST_WIDTH - BORDER_WIDTH);
    const float BT = B + IB;
    const float BXPL = static_cast<float>(BORDER_XPADDING_L);
    const float BXPR = static_cast<float>(BORDER_XPADDING_R);
    const float BYP = static_cast<float>(BORDER_YPADDING);
    const float W = static_cast<float>(clientRect.right);
    const float H = static_cast<float>(clientRect.bottom);


    // Outline of the size-able border area
    const D2D1_RECT_F borderOutsideRect
        = D2D1::RectF(BXPL, BYP, W - BXPR, float(clientRect.bottom) - BYP);

    // Rect of border area in the middle of the border width, used to actually draw the border
    const D2D1_RECT_F borderMiddleRect
        = D2D1::RectF(B / 2 + BXPL, B / 2 + BYP, W - B / 2 - BXPR, H - B / 2 - BYP);

    // Rect of inner border area in the middle of the border width 
    const D2D1_RECT_F innerBorderMiddleRect
        = D2D1::RectF(B + IB / 2 + BXPL, B + IB / 2 + BYP, W - B - IB / 2 - BXPR, H - B - IB / 2 - BYP);

    // Outline of inner content area
    const D2D1_RECT_F contentRect
        = D2D1::RectF(BT + BXPL, BT + BYP, W - BT - BXPR, H - BT - BYP);

    if (inEditMode())
    {
        m_pBrush->SetColor(EDIT_BORDER_COLOR);
        m_pRenderTarget->DrawRectangle(borderMiddleRect, m_pBrush, B, NULL);

        m_pBrush->SetColor(EDIT_INNER_BORDER_COLOR);
        m_pRenderTarget->DrawRectangle(innerBorderMiddleRect, m_pBrush, IB, NULL);
    }

    // Push a layer to "fade out" the right of the content area, using an opacity mask
    // First, set brush points to match window pixel size (gradient brushes do not scale)
    m_pLinearGradientBrush->SetStartPoint(D2D1::Point2F(float(clientRect.right - RIGHT_GRADIENT_WIDTH), float(clientRect.top)));
    m_pLinearGradientBrush->SetEndPoint(D2D1::Point2F(float(clientRect.right), float(clientRect.top)));
    m_pRenderTarget->PushLayer(
        D2D1::LayerParameters(
            D2D1::InfiniteRect(),
            NULL,
            D2D1_ANTIALIAS_MODE_PER_PRIMITIVE,
            D2D1::IdentityMatrix(),
            1.0f,
            m_pLinearGradientBrush,
            D2D1_LAYER_OPTIONS_NONE
        ),
        m_pContentLayer);

    if (inEditMode())
    {
        // Fill content area, fading out with the opacity mask
        m_pBrush->SetColor(EDIT_FILL_COLOR);
        m_pRenderTarget->FillRectangle(contentRect, m_pBrush);
    }


    // Set scaling transform
    m_pRenderTarget->SetTransform(
        D2D1::Matrix3x2F::Scale(D2D1::SizeF(m_scale, m_scale), D2D1::Point2F(0, 0)));

    if (inEditMode())
    {
        // Draw instruction text, centered in window
        m_pBrush->SetColor(EDIT_INNER_BORDER_COLOR);
        DWRITE_TEXT_METRICS dtm;
        throw_if_fail(m_pTextLayout->GetMetrics(&dtm));
        
        const auto origin = D2D1::Point2F(0, 0);
        m_pTextLayout->SetMaxWidth(W / calculateScale());
        m_pTextLayout->SetMaxHeight(H / 2 / calculateScale());
        m_pRenderTarget->DrawTextLayout(origin, m_pTextLayout, m_pBrush);
    }

    // Draw the content
    std::for_each(m_keyStrokeItems.begin(), m_keyStrokeItems.end(),
        std::bind(&KeyStrokeItem::draw, std::placeholders::_1, m_pRenderTarget, m_pItemLayer, m_pBrush));

    m_pRenderTarget->PopLayer();

}


// Cleanup done periodically, not at any particular time
// (currently LayeredWindow::tickFrame() calls this)
void KeyWindow::periodicUpdate()
{
    auto & v = m_keyStrokeItems;

    // Remove expired KeyStrokeItems from the list
    auto r = std::remove_if(v.begin(), v.end(), std::bind(&KeyStrokeItem::isDone, std::placeholders::_1));
    v.erase(r, v.end());

    // Poll key states (for active items only) and start the outro for keys which no longer read as down
    // (This can happen when keyboard focus switches to an admin window mid-press) 
    removeStuckItems();

}

void KeyWindow::addNewItem(UINT vkCode, UINT scanCode)
{
    try {
        // Construct k first, instead of emplacing it, as we need the 
        // finished object to calculate where to insert it.
        KeyStrokeItem k(vkCode, scanCode, m_pGraphicsManager);
        k.keyDown();

        auto newPos = m_keyStrokeItems.insert(findNewPosition(k), k);

        if (BEHAVIOUR_MOVE_FOR_MODIFIERS) {
            reCalculateLayout();
        }

        Debug::out << "New KeyStrokeItem " << vkCode
            << " at " << std::distance(m_keyStrokeItems.begin(), newPos)
            << " x=" << (*newPos).targetX() << std::endl;
    }
    catch (KeyStrokeItemConstructionException)
    {
        Debug::out << "Unhandled key " << vkCode << std::endl;
    }
}


void KeyWindow::handleKeyDown(UINT vkCode, UINT scanCode)
{
    if (!IsWindowVisible(m_hwnd)) {
        return;
    }

    auto & v = m_keyStrokeItems;

    // Check if this key is already shown as an item
    auto found = std::find_if(v.begin(), v.end(),
        [&vkCode](KeyStrokeItem const& k)
    {
        return (k.vkCode() == vkCode && k.isKeyDown());
    });

    if (found != v.end()) {
        // The key is still down; this is a repeat.
        // Ignore
    }

    else {
        // Find the first item for this key that's not down
        found = std::find_if(v.begin(), v.end(),
            [&vkCode](KeyStrokeItem const& k)
        {
            return (k.vkCode() == vkCode);
        });
        if (found != v.end()) {
            // The key is shown as an item

            if (BEHAVIOUR_REUSE) {
                // The keyStrokeItem is still shown but is already up. Reuse the item.
                (*found).keyDown();
            }
            else {
                // Add a new keyitem anyway.
                addNewItem(vkCode, scanCode);
            }
        }
        else
        {
            // No item exists for this key
            addNewItem(vkCode, scanCode);
        }
    }

}


void KeyWindow::handleKeyUp(UINT vkCode, UINT scanCode)
{
    DBG_UNREFERENCED_PARAMETER(scanCode);

    // Schedule outrok
    // Find the active keyStrokeItem for this key (There should be at most one)
    auto found = std::find_if(m_keyStrokeItems.begin(), m_keyStrokeItems.end(),
        [&vkCode](KeyStrokeItem const& k)
    {
        return (k.vkCode() == vkCode) && (k.isKeyDown());
    });

    if (found != m_keyStrokeItems.end()) {
        (*found).keyUp();
    }

    // Debug: make sure there are no more active items
    assert(
        std::count_if(m_keyStrokeItems.begin(), m_keyStrokeItems.end(),
            [&vkCode](KeyStrokeItem const& k) { return (k.vkCode() == vkCode) && (k.isKeyDown()); }
        ) == 0
    );

    Debug::out << "key up " << vkCode << std::endl;

}

// Find a position in the vector where the new item can fit (visually)
// newItem should be a newly initialized KeyStrokeItem not yet in the vector
std::vector<KeyStrokeItem>::const_iterator KeyWindow::findNewPosition(KeyStrokeItem &newItem)
{

    // Find element with enough room before it.
    const double neededWidth = newItem.width();
    double prevWidth = BASE_X;
    auto insertPosition = std::find_if(m_keyStrokeItems.begin(), m_keyStrokeItems.end(),
        [neededWidth, &prevWidth](KeyStrokeItem &k) mutable
    {
        const double kX = k.targetX();
        const double gapWidth = kX - prevWidth;

        if (gapWidth >= neededWidth) {
            return true;
        }

        // Ignore items which are already fading -
        // count the width only of active items.
        if (!(k.obscurable())) {
            prevWidth = kX + k.width();
        }

        return false;
    });


    newItem.setX(prevWidth);
    return insertPosition;
}


// Move the KeyStrokeItems horizontally so they don't overlap
// Put more recent keys on the right, except for modifier keys.
void KeyWindow::reCalculateLayout() {
    auto & v = m_keyStrokeItems;
    double prevWidth = BASE_X;

    // Sort using operator< defined by KeyStrokeItem
    std::stable_sort(v.begin(), v.end());

    // Reassign x to each item
    std::for_each(v.begin(), v.end(), [prevWidth, back = &v.back()](KeyStrokeItem &k) mutable
    {
        // Don't move or count fading items
        if (!k.obscurable()) {
            k.setX(prevWidth);
            prevWidth += k.width();
        }
    });

}

// Go through all keyStrokeItems and remove those where key is already up
void KeyWindow::removeStuckItems() {
    auto & v = m_keyStrokeItems;
    std::for_each(v.begin(), v.end(), [](KeyStrokeItem &k)
    {
        const auto state = GetAsyncKeyState(k.vkCode());
        const bool keyDown = (state & 0x8000) != 0;
        if (!keyDown) {
            k.startOutro();
        }
    });

}

// Simulate a window with resizable border
// This method should only be called when cx and cy are in the client area
LRESULT KeyWindow::clientHitTest(int cx, int cy)
{
    RECT r;
    GetClientRect(m_hwnd, &r);
    const int CW = r.right;
    const int CH = r.bottom;

    const auto & B = BORDER_HITTEST_WIDTH;
    const auto & XPL = BORDER_XPADDING_L;
    const auto & XPR = BORDER_XPADDING_R;
    const auto & YP = BORDER_YPADDING;

    // We pretend the client area is smaller than it really is.
    // There is a visible, draggable content area, surrounded by a sizable
    // border area, surrounded by an invisible area.

    if (cx < XPL || cy < YP || cx > CW - XPR || cy > CH - YP) {
        // Outside the border area
        // This should be transparent (alpha=0) so I don't expect this to be reached
        return HTNOWHERE;
    }

    // These coordinates start at the corner of the border rect
    const int x = cx - BORDER_XPADDING_L;
    const int y = cy - BORDER_YPADDING;
    const int W = CW - BORDER_XPADDING_L - BORDER_XPADDING_R;
    const int H = CH - 2 * BORDER_YPADDING;

    if (y < B) {
        if (x < B) return HTTOPLEFT;
        else if (x > W - B) return HTTOPRIGHT;
        else return HTTOP;
    }
    else if (y > H - B) {
        if (x < B) return HTBOTTOMLEFT;
        else if (x > W - B) return HTBOTTOMRIGHT;
        else return HTBOTTOM;
    }
    else {
        if (x < B) return HTLEFT;
        else if (x > W - B) return HTRIGHT;
    }
    return HTCAPTION;
}




