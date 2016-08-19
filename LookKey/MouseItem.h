#pragma once
#include "Graphics.h"
#include "Debug.h"

const UI_ANIMATION_SECONDS CLICK_DOWN_DURATION = 0.15;
const UI_ANIMATION_SECONDS CLICK_UP_DURATION = 0.5;
const UI_ANIMATION_SECONDS CLICK_DELAY_TIME = 0.01;
const DOUBLE CLICK_RIPPLE_MAX_RADIUS = 40;
const DOUBLE CLICK_RIPPLE_MAX_RADIUS_R = 40;
const DOUBLE CLICK_RIPPLE_MAX_RADIUS_M = 25;
const DOUBLE CLICK_RIPPLE_MAX_RADIUS_W = 18;
const DOUBLE CLICK_HANDLE_RADIUS = 10;
const D2D1_POINT_2F CLICK_HANDLE_OFFSET_L = { 0, 0 };
const D2D1_POINT_2F CLICK_HANDLE_OFFSET_R = { 3, 0 };
const D2D1_POINT_2F CLICK_HANDLE_OFFSET_M = { 1.5, -3 };
const FLOAT CLICK_WHEEL_OFFSET = 5;
const FLOAT CLICK_RIPPLE_THICKNESS = 2;
const FLOAT CLICK_RIPPLE_THICKNESS_R = 2;
const FLOAT CLICK_RIPPLE_THICKNESS_M = 3;
const FLOAT CLICK_RIPPLE_THICKNESS_W = 4;
const FLOAT CLICK_GRADIENT_WIDTH = 5;
const D2D1_COLOR_F CLICK_COLOR_L = D2D1::ColorF(D2D1::ColorF::DarkBlue, 1.0f);
const D2D1_COLOR_F CLICK_COLOR_R = D2D1::ColorF(D2D1::ColorF::Firebrick, 1.0f);
const D2D1_COLOR_F CLICK_COLOR_M = D2D1::ColorF(D2D1::ColorF::DarkGreen, 1.0f);
const D2D1_COLOR_F CLICK_COLOR_W = D2D1::ColorF(D2D1::ColorF::DarkGreen, 1.0f);
const FLOAT CLICK_HANDLE_OPACITY = 0.5f;

class MouseItem
{
protected:
    MouseItem(UINT vkCode, GraphicsManager * graphicsManager);
public:
    virtual ~MouseItem() {}
    void startDown(POINT pt);
    void startUp(POINT pt);
    void updatePos(POINT pt);
    bool isActive() const;
    bool isDone() const;
    UINT getVkCode();
    virtual void draw(ID2D1RenderTarget * renderTarget,
        ID2D1Layer * layer,
        ID2D1SolidColorBrush * solidColorBrush,
        ID2D1LinearGradientBrush * linearGradientBrush) = 0;

private:
    void initStoryboards();

    // Animation pointers
    CComPtr<IUIAnimationManager> m_pAnimationManager;
    CComPtr<IUIAnimationTimer> m_pAnimationTimer;
    CComPtr<IUIAnimationTransitionLibrary> m_pTransitionLibrary;

protected:
    // State variables
    D2D1_POINT_2F m_origin;
    D2D1_POINT_2F m_pos;
    UINT m_vkCode;

    // Animation variables
    CComPtr<IUIAnimationVariable> m_pAnimationVariableR;
    CComPtr<IUIAnimationVariable> m_pAnimationVariableHandle;

    CComPtr<IUIAnimationStoryboard> m_pStoryboardDown;
    CComPtr<IUIAnimationStoryboard> m_pStoryboardUp;
};

class MouseLButtonItem : public MouseItem
{
public:
    MouseLButtonItem(GraphicsManager * gm) :
        MouseItem(VK_LBUTTON, gm) {}
    ~MouseLButtonItem() {}

    void draw(ID2D1RenderTarget * renderTarget,
        ID2D1Layer * layer,
        ID2D1SolidColorBrush * solidColorBrush,
        ID2D1LinearGradientBrush * linearGradientBrush);
};

class MouseRButtonItem : public MouseItem
{
public:
    MouseRButtonItem(GraphicsManager * gm) :
        MouseItem(VK_RBUTTON, gm) {}
    ~MouseRButtonItem() {}

    void draw(ID2D1RenderTarget * renderTarget,
        ID2D1Layer * layer,
        ID2D1SolidColorBrush * solidColorBrush,
        ID2D1LinearGradientBrush * linearGradientBrush);
};

class MouseMButtonItem : public MouseItem
{
public:
    MouseMButtonItem(GraphicsManager * gm) :
        MouseItem(VK_MBUTTON, gm) {}
    ~MouseMButtonItem() {}

    void draw(ID2D1RenderTarget * renderTarget,
        ID2D1Layer * layer,
        ID2D1SolidColorBrush * solidColorBrush,
        ID2D1LinearGradientBrush * linearGradientBrush);

};

class MouseWheelItem : public MouseItem
{
public:
    MouseWheelItem(GraphicsManager * gm, POINT dir) :
        MouseItem(NULL, gm), m_direction(dir) {}
    ~MouseWheelItem() {}

    void draw(ID2D1RenderTarget * renderTarget,
        ID2D1Layer * layer,
        ID2D1SolidColorBrush * solidColorBrush,
        ID2D1LinearGradientBrush * linearGradientBrush);

private:
    POINT m_direction;
};