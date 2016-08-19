#include "stdafx.h"
#include "LayeredWindow.h"

#include "LookKey.h"
#include "Config.h"
#include "KeyStrokeItem.h"
#include "RenderTargetDC.h"

Debug::FpsCounter LayeredWindow::m_fpsCounter{};

LayeredWindow::LayeredWindow(PCWSTR classname, GraphicsManager * graphicsManager, ConfigManager * config) :
    BaseWindow(classname),
    m_pGraphicsManager(graphicsManager),
    m_pConfigManager(config)
{

}

// Two-phase initialization: init() should be called in derived constructor,
// so derived can handle messages during window creation 
void LayeredWindow::init(int x, int y, int width, int height)
{
    createDeviceResources();

    WCHAR title[100]; // The title bar text
    LoadStringW(GetModuleHandle(NULL), IDS_APP_TITLE, title, ARRAYSIZE(title));

    // Register the window class and reate the Window
    throw_if_fail(
        Create(title,
            WS_POPUP,
            WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
            {}, //wndClass
            x, y,
            width, height,
            NULL) // parent 
    );


    // Create timer to render on WM_TIMER
    throw_if_fail(
        SetTimer(m_hwnd, NULL, 10, NULL) != 0
    );

    handleResize();
    
}


LRESULT LayeredWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TIMER:
    {
        tickFrame();
        break;
    }
    case WM_WINDOWPOSCHANGED: {
        handleResize();
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
    case WM_STYLECHANGING:
    {
        LONG l = GetWindowLong(this->hWnd(), GWL_EXSTYLE);
        BOOL a = l & WS_EX_LAYERED;
        a = l & WS_EX_TOOLWINDOW;
        //SetWindowLong(m_pKeyWin->hWnd(), GWL_EXSTYLE, l);
        Debug::out << m_classname << " WM_STYLECHANGING: " << -signed(wParam)
            << " toolwindow: " << (a!=0) << std::endl;
        break;
    }
    case WM_STYLECHANGED:
    {
        LONG l = GetWindowLong(this->hWnd(), GWL_EXSTYLE);
        BOOL a = l & WS_EX_LAYERED;
        a = l & WS_EX_TOOLWINDOW;
        //SetWindowLong(m_pKeyWin->hWnd(), GWL_EXSTYLE, l);
        Debug::out << m_classname << " WM_STYLECHANGED: " << -signed(wParam)
            << " toolwindow: " << (a != 0) << std::endl;
        break;
    }
    case WM_DISPLAYCHANGE: {
        int resx = GET_X_LPARAM(lParam);
        int resy = GET_Y_LPARAM(lParam);

        Debug::out << "WM_DISPLAYCHANGE: " << resx << " x " << resy << std::endl;
        break;
    }
    default:
        return DefWindowProc(m_hwnd, message, wParam, lParam);
    }
    return 0;
}

void LayeredWindow::setEnabled(bool enabled)
{
    if (enabled) {
        SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    else {
        ShowWindow(m_hwnd, SW_HIDE);
    }

    Debug::out << m_classname << " enabled: " << enabled << std::endl;
}

BOOL LayeredWindow::isEnabled()
{
    return IsWindowVisible(m_hwnd);
}

void LayeredWindow::discardDeviceResources()
{
    m_pRenderTarget.Release();
    m_pGdiInteropRenderTarget.Release();
}

// Create device-dependent resources
void LayeredWindow::createDeviceResources()
{
    if (!m_pWICbitmap) {
        CComPtr<IWICImagingFactory> pWICFactory;
        m_pGraphicsManager->getWICImagingFactory(&pWICFactory);

        // Create WIC bitmap of tiny size to start
        // TODO: there may be a better way!
        throw_if_fail(pWICFactory->CreateBitmap(
            1,
            1,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapCacheOnLoad,
            &m_pWICbitmap));
    }

    if (!m_pRenderTarget) {
        CComPtr<ID2D1Factory> pD2DFactory;
        m_pGraphicsManager->getD2DFactory(&pD2DFactory);

        const D2D1_RENDER_TARGET_PROPERTIES properties =
            D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_DEFAULT,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                0, 0, //default dpi
                D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE);

        throw_if_fail(
            pD2DFactory->CreateWicBitmapRenderTarget(m_pWICbitmap, properties, &m_pRenderTarget)
        );
        throw_if_fail(m_pRenderTarget.QueryInterface(&m_pGdiInteropRenderTarget));
    }


    // The hard stuff is done.
    // Create D2D brushes, layers, and other such things
    if (!m_pBrush) {
        throw_if_fail(
            m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0, 0), &m_pBrush)
        );
    }

    if (!m_pLinearGradientStopCollection) {
        D2D1_GRADIENT_STOP stops[] = { { 0.0f, D2D1::ColorF(1,1,1,1) },
        { 1.0f, D2D1::ColorF(1,1,1,1.0f / 300) } };
        throw_if_fail(m_pRenderTarget->CreateGradientStopCollection(
            stops,
            ARRAYSIZE(stops),
            D2D1_GAMMA_2_2,
            D2D1_EXTEND_MODE_CLAMP,
            &m_pLinearGradientStopCollection
        ));
    }
    if (!m_pLinearGradientBrush) {
        throw_if_fail(m_pRenderTarget->CreateLinearGradientBrush(
        {},
            m_pLinearGradientStopCollection,
            &m_pLinearGradientBrush
        ));
    }

    if (!m_pItemLayer) {
        // Create layer, reused for each item
        throw_if_fail(m_pRenderTarget->CreateLayer(&m_pItemLayer));
    }
    if (!m_pContentLayer) {
        // Create layer, used for alpha mask of entire content area
        throw_if_fail(m_pRenderTarget->CreateLayer(&m_pContentLayer));
    }

}



void LayeredWindow::handleResize()
{
    RECT clientRect, windowRect;
    throw_if_fail(
        GetClientRect(m_hwnd, &clientRect)
    );
    throw_if_fail(
        GetWindowRect(m_hwnd, &windowRect)
    );

    const int x = windowRect.left; // Window x position
    const int y = windowRect.top; // Window y position
    const int cx = clientRect.right; // Client width
    const int cy = clientRect.bottom; // Client height

    m_pos = { x, y };
    m_dirty = true; // We need to redraw

    if (cx != m_size.cx || cy != m_size.cy)
    {
        // Update instance state
        m_size = { cx, cy };
        m_scale = calculateScale();

        discardDeviceResources();

        CComPtr<IWICImagingFactory> pWICFactory;
        m_pGraphicsManager->getWICImagingFactory(&pWICFactory);

        // Create WIC bitmap of the correct size
        m_pWICbitmap.Release();
        throw_if_fail(pWICFactory->CreateBitmap(
            m_size.cx,
            m_size.cy,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapCacheOnLoad,
            &m_pWICbitmap));
    }

}

// Default implementation  
float LayeredWindow::calculateScale() const {
    return 1.0f;
}

void LayeredWindow::tickFrame()
{
    // Any non-graphical updates in derived class
    periodicUpdate();

    if (!IsWindowVisible(m_hwnd)) {
        return;
    }

    // Update the animation manager so it calculates the next frame
    m_pGraphicsManager->updateAnimationManagerTime();

    bool isBusy = 0;
    m_pGraphicsManager->isAnimationManagerBusy(&isBusy);

    if (isBusy || m_dirty)
    {
        // Create resources if needed
        createDeviceResources();

        m_pRenderTarget->BeginDraw();

        drawClientArea();

        // Update the window
        // Use a scope to destruct RenderTargetDC on throw
        {
            RenderTargetDC renderTargetDC(m_pGdiInteropRenderTarget);
            BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
            POINT zeroPoint = { 0, 0 };

            throw_if_fail(
                UpdateLayeredWindow(m_hwnd,
                    NULL, // screen DC, not needed
                    &m_pos, // new window pos
                    &m_size, // new window size
                    renderTargetDC, // DC defining window
                    &zeroPoint, // location of layer in DC
                    NULL, // color key, not using 
                    &blend,
                    ULW_ALPHA) // use blend function, not color key
            );
        }

        // Here is where any errors during the drawing pop up
        try
        {
            throw_if_fail(m_pRenderTarget->EndDraw());
        }
        catch (_com_error err)
        {
            if (err.Error() == D2DERR_RECREATE_TARGET) {
                discardDeviceResources();
            }
            else {
                throw err;
            }
        }

        m_fpsCounter.countFrame();

    }

    if (m_fpsCounter.getFrameCount() > 60)
    {
        Debug::out << m_fpsCounter.getFps() << "FPS" << std::endl;
        m_fpsCounter.reset();
    }
    // We need to draw one more frame after the animation manager finishes,
    // to ensure the final state is on the screen - so use the m_dirty flag.
    // (m_dirty is also used elsewhere - otherwise it'd be a static local)
    m_dirty = isBusy;

}
