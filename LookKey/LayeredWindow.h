#pragma once
#include "BaseWindow.h"

#include "KeyStrokeItem.h"
#include "BaseWindow.h"
#include "Graphics.h"
#include "Config.h"
#include "Debug.h"


class LayeredWindow : public BaseWindow<LayeredWindow>
{
protected:
    LayeredWindow(PCWSTR classname, GraphicsManager * graphicsManager, ConfigManager * config);
    void init(int x, int y, int width, int height);

public:
    virtual ~LayeredWindow() {};
    LRESULT CALLBACK HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void setEnabled(bool enabled);
    BOOL isEnabled();

private:
    void discardDeviceResources();
    void createDeviceResources();
    void tickFrame();


protected:
    void handleResize();
    virtual void periodicUpdate() { };
    virtual void drawClientArea() = 0;
    virtual float calculateScale() const;

    // Helper objects
    GraphicsManager * m_pGraphicsManager;
    ConfigManager * m_pConfigManager;

    // State
    bool m_dirty = 0;
    float m_scale = 1;
    POINT m_pos;
    SIZE m_size;

    // Direct2D innards
    CComPtr<ID2D1RenderTarget> m_pRenderTarget;
    CComPtr<ID2D1GdiInteropRenderTarget> m_pGdiInteropRenderTarget;

    // WIC innards
    CComPtr<IWICBitmap> m_pWICbitmap;

    // Direct2D drawing resources
    CComPtr<ID2D1SolidColorBrush> m_pBrush;
    CComPtr<ID2D1RectangleGeometry> m_pGeometry;
    CComPtr<ID2D1LinearGradientBrush> m_pLinearGradientBrush;
    CComPtr<ID2D1GradientStopCollection> m_pLinearGradientStopCollection;
    CComPtr<ID2D1Layer> m_pContentLayer;
    CComPtr<ID2D1Layer> m_pItemLayer;

    HDC m_dc;
    
    static Debug::FpsCounter m_fpsCounter;

};

