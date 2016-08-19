#include "stdafx.h"
#include "MouseItem.h"
#include "LookKey.h"


MouseItem::MouseItem(UINT vkCode, GraphicsManager * graphicsManager) :
    m_vkCode(vkCode)
{
    // Fetch animation pointers
    graphicsManager->getAnimationManager(&m_pAnimationManager);
    graphicsManager->getAnimationTransitionLibrary(&m_pTransitionLibrary);
    graphicsManager->getAnimationTimer(&m_pAnimationTimer);

    //Create animation variables
    throw_if_fail(m_pAnimationManager->CreateAnimationVariable(CLICK_RIPPLE_MAX_RADIUS, &m_pAnimationVariableR));
    throw_if_fail(m_pAnimationManager->CreateAnimationVariable(0.0, &m_pAnimationVariableHandle));


    // Create intro storyboard and transitions. Don't schedule yet.
    initStoryboards();
}

void MouseItem::startDown(POINT pt)
{
    m_pos = m_origin = { float(pt.x), float(pt.y) };

    UI_ANIMATION_SECONDS secondsNow;
    throw_if_fail(m_pAnimationTimer->GetTime(&secondsNow));
    throw_if_fail(m_pStoryboardDown->Schedule(secondsNow));
}

void MouseItem::startUp(POINT pt)
{
    m_origin = { float(pt.x), float(pt.y) };
    UI_ANIMATION_SECONDS secondsNow;
    throw_if_fail(m_pStoryboardDown->Finish(CLICK_DELAY_TIME));
    throw_if_fail(m_pAnimationTimer->GetTime(&secondsNow));
    throw_if_fail(m_pStoryboardUp->Schedule(secondsNow));
}



void MouseItem::initStoryboards()
{
    // Storyboard Down
    // When button is pressed down and held
    throw_if_fail(m_pAnimationManager->CreateStoryboard(&m_pStoryboardDown));

    // TransitionR_Decrease: shrink radius linearly, while "handle" grows to 1 (normalized)
    CComPtr<IUIAnimationTransition> pTransitionR_Decrease;
    CComPtr<IUIAnimationTransition> pTransitionLinearTo1;
    throw_if_fail(m_pTransitionLibrary->CreateLinearTransition(CLICK_DOWN_DURATION, 0, &pTransitionR_Decrease));
    throw_if_fail(m_pStoryboardDown->AddTransition(m_pAnimationVariableR, pTransitionR_Decrease));
    throw_if_fail(m_pTransitionLibrary->CreateLinearTransition(CLICK_DOWN_DURATION, 1.0, &pTransitionLinearTo1));
    throw_if_fail(m_pStoryboardDown->AddTransition(m_pAnimationVariableHandle, pTransitionLinearTo1));

    // Then hold constant
    throw_if_fail(m_pStoryboardDown->HoldVariable(m_pAnimationVariableR));
    throw_if_fail(m_pStoryboardDown->HoldVariable(m_pAnimationVariableHandle));


    // Storyboard Up
    // When button is released
    throw_if_fail(m_pAnimationManager->CreateStoryboard(&m_pStoryboardUp));

    // TransitionR_Increase: Expand radius linearly, hide handle quickly
    CComPtr<IUIAnimationTransition> pTransitionR_Increase;
    CComPtr<IUIAnimationTransition> pTransitionLinearTo0;

    throw_if_fail(m_pTransitionLibrary->CreateLinearTransition(CLICK_UP_DURATION, CLICK_RIPPLE_MAX_RADIUS, &pTransitionR_Increase));
    throw_if_fail(m_pStoryboardUp->AddTransition(m_pAnimationVariableR, pTransitionR_Increase));
    throw_if_fail(m_pTransitionLibrary->CreateLinearTransition(CLICK_DELAY_TIME, 0, &pTransitionLinearTo0));
    throw_if_fail(m_pStoryboardUp->AddTransition(m_pAnimationVariableR, pTransitionLinearTo0));

    throw_if_fail(m_pStoryboardUp->SetLongestAcceptableDelay(CLICK_DELAY_TIME));

}

bool MouseItem::isActive() const
{
    // TODO: add storyboard event handlers 
    UI_ANIMATION_STORYBOARD_STATUS status;
    throw_if_fail(m_pStoryboardUp->GetStatus(&status));

    return (status == UI_ANIMATION_STORYBOARD_BUILDING); // has never been scheduled
}

bool MouseItem::isDone() const
{
    // TODO: add storyboard event handlers 
    UI_ANIMATION_STORYBOARD_STATUS status;
    throw_if_fail(m_pStoryboardUp->GetStatus(&status));

    return (status == UI_ANIMATION_STORYBOARD_READY); // has finished playing
}

UINT MouseItem::getVkCode()
{
    return m_vkCode;
}

// Draw a single frame, assuming beginDraw() has already been called.
void MouseLButtonItem::draw(ID2D1RenderTarget * renderTarget,
    ID2D1Layer * layer,
    ID2D1SolidColorBrush * solidColorBrush,
    ID2D1LinearGradientBrush * linearGradientBrush)
{
    DBG_UNREFERENCED_PARAMETER(layer);

    // Fetch animation variable values
    double r;
    throw_if_fail(m_pAnimationVariableR->GetValue(&r));

    if (r > 0)
    {
        const auto origin = m_origin;

        // Push a layer to hide one side of the ripple, using an opacity mask
        // First, set brush points to match ripple position
        linearGradientBrush->SetStartPoint(origin);
        linearGradientBrush->SetEndPoint({ origin.x + CLICK_GRADIENT_WIDTH, origin.y });

        D2D1_ELLIPSE circle = D2D1::Ellipse(origin, float(r), float(r));
        solidColorBrush->SetColor(CLICK_COLOR_L);
        solidColorBrush->SetOpacity(float(1 - r / CLICK_RIPPLE_MAX_RADIUS));

        renderTarget->DrawEllipse(circle, solidColorBrush, CLICK_RIPPLE_THICKNESS, NULL);
    }
}


// Draw a single frame, assuming beginDraw() has already been called.
void MouseRButtonItem::draw(ID2D1RenderTarget * renderTarget,
    ID2D1Layer * layer,
    ID2D1SolidColorBrush * solidColorBrush,
    ID2D1LinearGradientBrush * linearGradientBrush)
{
    // Fetch animation variable values
    double r;
    throw_if_fail(m_pAnimationVariableR->GetValue(&r));

    if (r > 0)
    {
        const auto origin = m_origin;

        // Push a layer to hide one side of the ripple, using an opacity mask
        // First, set brush points to match ripple position
        linearGradientBrush->SetStartPoint(origin);
        linearGradientBrush->SetEndPoint({ origin.x - CLICK_GRADIENT_WIDTH, origin.y });

        // Mask one side of ripple
        renderTarget->PushLayer(
            D2D1::LayerParameters(
                D2D1::InfiniteRect(),
                NULL,
                D2D1_ANTIALIAS_MODE_PER_PRIMITIVE,
                D2D1::IdentityMatrix(),
                1.0f,
                linearGradientBrush,
                D2D1_LAYER_OPTIONS_NONE
            ),
            layer); // TODO: layer not needed in Win8+

        D2D1_ELLIPSE circle = D2D1::Ellipse(origin, float(r), float(r));

        solidColorBrush->SetColor(CLICK_COLOR_R);
        solidColorBrush->SetOpacity(float(1 - r / CLICK_RIPPLE_MAX_RADIUS_R));
        renderTarget->DrawEllipse(circle, solidColorBrush, CLICK_RIPPLE_THICKNESS_R, NULL);

        renderTarget->PopLayer();
    }
}

// Draw a single frame, assuming beginDraw() has already been called.
void MouseMButtonItem::draw(ID2D1RenderTarget * renderTarget,
    ID2D1Layer * layer,
    ID2D1SolidColorBrush * solidColorBrush,
    ID2D1LinearGradientBrush * linearGradientBrush)
{
    DBG_UNREFERENCED_PARAMETER(layer);

    // Fetch animation variable values
    double r;
    throw_if_fail(m_pAnimationVariableR->GetValue(&r));

    if (r > 0)
    {
        const auto origin = m_origin;

        // Push a layer to hide one side of the ripple, using an opacity mask
        // First, set brush points to match ripple position
        linearGradientBrush->SetStartPoint(origin);
        linearGradientBrush->SetEndPoint({ origin.x + CLICK_GRADIENT_WIDTH, origin.y });

        D2D1_ELLIPSE circle = D2D1::Ellipse(origin, float(r), float(r));

        solidColorBrush->SetColor(CLICK_COLOR_M);
        solidColorBrush->SetOpacity(float(1 - r / CLICK_RIPPLE_MAX_RADIUS_M));
        renderTarget->DrawEllipse(circle, solidColorBrush, CLICK_RIPPLE_THICKNESS_M, NULL);
    }
}

void MouseWheelItem::draw(ID2D1RenderTarget * renderTarget,
    ID2D1Layer * layer,
    ID2D1SolidColorBrush * solidColorBrush,
    ID2D1LinearGradientBrush * linearGradientBrush)
{
    // Fetch animation variable values
    double r;
    throw_if_fail(m_pAnimationVariableR->GetValue(&r));

    if (r > 0)
    {
        const auto dir = m_direction;
        const D2D1_POINT_2F offset = { dir.x * CLICK_WHEEL_OFFSET,
                                       dir.y * CLICK_WHEEL_OFFSET };
        const auto origin = m_origin + offset;

        // Push a layer to hide one side of the ripple, using an opacity mask
        // First, set brush points to match ripple position
        linearGradientBrush->SetStartPoint(origin);
        linearGradientBrush->SetEndPoint({ origin.x - dir.x*CLICK_GRADIENT_WIDTH,
                                           origin.y - dir.y*CLICK_GRADIENT_WIDTH });

        // Mask one side of ripple
        renderTarget->PushLayer(
            D2D1::LayerParameters(
                D2D1::InfiniteRect(),
                NULL,
                D2D1_ANTIALIAS_MODE_PER_PRIMITIVE,
                D2D1::IdentityMatrix(),
                1.0f,
                linearGradientBrush,
                D2D1_LAYER_OPTIONS_NONE
            ),
            layer); // TODO: layer not needed in Win8+

        D2D1_ELLIPSE circle = D2D1::Ellipse(origin, float(r), float(r));

        solidColorBrush->SetColor(CLICK_COLOR_W);
        solidColorBrush->SetOpacity(float(1 - r / CLICK_RIPPLE_MAX_RADIUS_W));
        renderTarget->DrawEllipse(circle, solidColorBrush, CLICK_RIPPLE_THICKNESS_W, NULL);

        renderTarget->PopLayer();
    }
}
