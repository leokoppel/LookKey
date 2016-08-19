#include "stdafx.h"
#include "KeyStrokeItem.h"

#include "LookKey.h"
#include "KeyWindow.h"
#include "Graphics.h"

KeyStrokeItem::KeyStrokeItem(UINT vkCode, UINT scanCode, GraphicsManager * graphicsManager)
    :
    m_vkCode(vkCode),
    m_scanCode(scanCode)
{

    // Fetch animation pointers
    initText(graphicsManager);

    graphicsManager->getAnimationManager(&m_pAnimationManager);
    graphicsManager->getAnimationTransitionLibrary(&m_pTransitionLibrary);
    graphicsManager->getAnimationTimer(&m_pAnimationTimer);

    //Create animation variables
    throw_if_fail(m_pAnimationManager->CreateAnimationVariable(BASE_X, &m_pAnimationVariableX));
    throw_if_fail(m_pAnimationManager->CreateAnimationVariable(BASE_Y, &m_pAnimationVariableY));
    throw_if_fail(m_pAnimationManager->CreateAnimationVariable(INITIAL_ALPHA, &m_pAnimationVariableAlpha));

    // Create intro storyboard and transitions. Don't schedule yet.
    initStoryboards();
}

// Create storyboards without scheduling any
void KeyStrokeItem::initStoryboards()
{
    // Storyboard Intro
    // When key is pressed down
    CComPtr<IUIAnimationTransition> pTransitionY_Intro;
    CComPtr<IUIAnimationTransition> pTransitionAlpha_Intro;
    throw_if_fail(m_pAnimationManager->CreateStoryboard(&m_pStoryboardIntro));

    // TransitionY_Intro: Dip down below BASE_Y
    throw_if_fail(m_pTransitionLibrary->CreateLinearTransition(INTRO_DURATION, BASE_Y + INTRO_Y_DIP, &pTransitionY_Intro));
    throw_if_fail(m_pStoryboardIntro->AddTransition(m_pAnimationVariableY, pTransitionY_Intro));

    // TransitionAlpha_Intro: Set alpha back to opaque
    throw_if_fail(m_pTransitionLibrary->CreateLinearTransition(INTRO_DURATION, 1.0f, &pTransitionAlpha_Intro));
    throw_if_fail(m_pStoryboardIntro->AddTransition(m_pAnimationVariableAlpha, pTransitionAlpha_Intro));

    // Storyboard KeyUp
    // When key is lifted, but doesn't yet disappear
    CComPtr<IUIAnimationTransition> pTransitionY_KeyUp;
    CComPtr<IUIAnimationTransition> pTransitionAlpha_Hold;
    CComPtr<IUIAnimationTransition> pTransitionY_Hold;
    throw_if_fail(m_pAnimationManager->CreateStoryboard(&m_pStoryboardKeyUp));

    // TransitionY_KeyUp: Move item back to BASE_Y
    throw_if_fail(m_pTransitionLibrary->CreateLinearTransition(KEYUP_DURATION, BASE_Y, &pTransitionY_KeyUp));
    throw_if_fail(m_pStoryboardKeyUp->AddTransition(m_pAnimationVariableY, pTransitionY_KeyUp));

    // TransitionY_Hold: Hold for HOLD_DURATION after key is lifted
    throw_if_fail(m_pTransitionLibrary->CreateConstantTransition(HOLD_DURATION, &pTransitionY_Hold));
    throw_if_fail(m_pStoryboardKeyUp->AddTransition(m_pAnimationVariableY, pTransitionY_Hold));

    // TransitionAlpha_Hold: Hold alpha during pTransitionY_KeyUp and pTransitionY_Hold
    throw_if_fail(m_pTransitionLibrary->CreateConstantTransition(KEYUP_DURATION + HOLD_DURATION, &pTransitionAlpha_Hold));
    throw_if_fail(m_pStoryboardKeyUp->AddTransition(m_pAnimationVariableAlpha, pTransitionAlpha_Hold));


    // Storyboard Outro
    // After hold duration is complete
    CComPtr<IUIAnimationTransition> pTransitionY_Outro;
    CComPtr<IUIAnimationTransition> pTransitionAlpha_Outro;
    throw_if_fail(m_pAnimationManager->CreateStoryboard(&m_pStoryboardOutro));

    // TransitionY_Outro: Raise item by OUTRO_Y_RISE
    throw_if_fail(m_pTransitionLibrary->CreateLinearTransition(OUTRO_DURATION, BASE_Y - OUTRO_Y_RISE, &pTransitionY_Outro));
    throw_if_fail(m_pStoryboardOutro->AddTransition(m_pAnimationVariableY, pTransitionY_Outro));

    // TransitionAlpha_Outro: Fade out item as it rises
    throw_if_fail(m_pTransitionLibrary->CreateLinearTransition(OUTRO_DURATION, 0, &pTransitionAlpha_Outro));
    throw_if_fail(m_pStoryboardOutro->AddTransition(m_pAnimationVariableAlpha, pTransitionAlpha_Outro));


    // Don't interrupt keyup/hold storyboard with outro, even if it's scheduled immediately 
    throw_if_fail(m_pStoryboardOutro->SetLongestAcceptableDelay(KEYUP_DURATION + HOLD_DURATION));

}

KeyStrokeItem::~KeyStrokeItem()
{
    Debug::out << "destroying " << this << std::endl;
}

void KeyStrokeItem::initText(GraphicsManager * graphicsManager)
{

    // Turn vkCode and scanCode into a name for the key
    HRESULT hr = getKeyString(m_vkCode, m_scanCode, m_str, MAX_KEY_NAME_LEN);
    if (FAILED(hr)) {
        // This key is not one we can handle
        throw KeyStrokeItemConstructionException();
    }

    // Initialize text layout for this instance
    // Layout is scaled with transform, so can use default window width as maxWidth
    // Height doesn't matter as text is never clipped vertically
    const float maxWidth = float(Config::getDefaultKeyWindowWidth());
    const float maxHeight = 0.0f; // 
    CComPtr<IDWriteFactory> pDWriteFactory;
    CComPtr<IDWriteTextFormat> pTextFormat;

    graphicsManager->getDWriteFactory(&pDWriteFactory);
    graphicsManager->getDWriteTextFormat(&pTextFormat);

    throw_if_fail(pDWriteFactory->CreateTextLayout(
        m_str,
        static_cast<UINT32>(wcslen(m_str)), // Warning: technically unsafe int64->int32 
        pTextFormat,
        maxWidth,
        maxHeight,
        &m_pTextLayout
    ));


    // Find width of text.
    // In case key name contains spaces, we want it to stay on one line.
    // That's why we use a wide initial layout size and GetMetrics over DetermineMinWidth
    DWRITE_TEXT_METRICS dtm;
    throw_if_fail(m_pTextLayout->GetMetrics(&dtm));

    // Box must be at least as wide as it is tall
    m_textWidth = max(dtm.width, float(BOX_HEIGHT));

    // Resize layout so text will be centred within box
    throw_if_fail(m_pTextLayout->SetMaxWidth(m_textWidth));
    throw_if_fail(m_pTextLayout->SetMaxHeight(float(BOX_HEIGHT)));
}

void KeyStrokeItem::setInstantX(DOUBLE x)
{
    m_targetX = x;
    UI_ANIMATION_SECONDS secondsNow;
    CComPtr<IUIAnimationTransition> pTransitionX;

    if (!m_set_X)
    {
        m_set_X = true;
    }

    throw_if_fail(m_pAnimationTimer->GetTime(&secondsNow));

    throw_if_fail(m_pTransitionLibrary->CreateInstantaneousTransition(m_targetX, &pTransitionX));

    throw_if_fail(m_pAnimationManager->ScheduleTransition(m_pAnimationVariableX, pTransitionX, secondsNow));

}


void KeyStrokeItem::setX(DOUBLE x)
{
    // Set target X and start a smooth transition.

    if (!m_set_X)
    {
        return setInstantX(x);
    }

    if (x != m_targetX)
    {
        m_targetX = x;
        UI_ANIMATION_SECONDS secondsNow;
        CComPtr<IUIAnimationTransition> pTransitionX;
        throw_if_fail(m_pAnimationTimer->GetTime(&secondsNow));
        throw_if_fail(m_pTransitionLibrary->CreateAccelerateDecelerateTransition(
            X_MOVE_DURATION,
            m_targetX,
            X_MOVE_ACCEL,
            X_MOVE_DECCEL,
            &pTransitionX
        ));

        throw_if_fail(m_pAnimationManager->ScheduleTransition(m_pAnimationVariableX, pTransitionX, secondsNow));
    }

}


void KeyStrokeItem::keyDown()
{
    UI_ANIMATION_SECONDS secondsNow;

    m_pStoryboardIntro->Abandon();
    m_pStoryboardKeyUp->Abandon();
    m_pStoryboardOutro->Abandon();


    throw_if_fail(m_pAnimationTimer->GetTime(&secondsNow));
    throw_if_fail(m_pStoryboardIntro->Schedule(secondsNow));

    m_keyDown = true;

}



void KeyStrokeItem::keyUp()
{
    UI_ANIMATION_SECONDS secondsNow;
    throw_if_fail(m_pAnimationTimer->GetTime(&secondsNow));
    throw_if_fail(m_pStoryboardKeyUp->Schedule(secondsNow));

    startOutro();

    m_keyDown = false;
}

void KeyStrokeItem::startOutro()
{
    UI_ANIMATION_SECONDS secondsNow;
    throw_if_fail(m_pAnimationTimer->GetTime(&secondsNow));
    throw_if_fail(m_pStoryboardOutro->Schedule(secondsNow));
}

void KeyStrokeItem::draw(ID2D1RenderTarget * renderTarget, ID2D1Layer * layer, ID2D1SolidColorBrush * solidColorBrush)
{
    DOUBLE x, y, alpha;
    assert(renderTarget);

    // Fetch animation variable values
    throw_if_fail(m_pAnimationVariableX->GetValue(&x));
    throw_if_fail(m_pAnimationVariableY->GetValue(&y));
    throw_if_fail(m_pAnimationVariableAlpha->GetValue(&alpha));

    // Draw a single frame, assuming beginDraw() has already been called.
        // First we push a layer, here setting the opacity of the item
    renderTarget->PushLayer(
        D2D1::LayerParameters(
            D2D1::InfiniteRect(),
            NULL,
            D2D1_ANTIALIAS_MODE_PER_PRIMITIVE,
            D2D1::IdentityMatrix(),
            static_cast<FLOAT>(alpha*BOX_ALPHA), // opacity
            NULL,
            D2D1_LAYER_OPTIONS_NONE),
        layer
    );

    // Draw box
    D2D1_RECT_F boxRect = D2D1::RectF(
        static_cast<FLOAT>(x),
        static_cast<FLOAT>(y),
        static_cast<FLOAT>(x + m_textWidth + 2 * (BOX_PADDING + BOX_BORDER)),
        static_cast<FLOAT>(y + BOX_HEIGHT + 2 * (BOX_PADDING + BOX_BORDER))
    );

    D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(boxRect, BOX_CORNER_RADIUS, BOX_CORNER_RADIUS);

    solidColorBrush->SetColor(BOX_COLOR);
    renderTarget->FillRoundedRectangle(roundedRect, solidColorBrush);

    // Draw border
    solidColorBrush->SetColor(BOX_BORDER_COLOR);
    renderTarget->DrawRoundedRectangle(roundedRect, solidColorBrush, BOX_BORDER, NULL);


    // Set origin of text at top left of box's "content area"
    // (inside padding and border - note border drawn half outside filled rect)
    D2D1_POINT_2F origin = D2D1::Point2F(
        static_cast<FLOAT>(x + BOX_PADDING + BOX_BORDER / 2),
        static_cast<FLOAT>(y + BOX_PADDING + BOX_BORDER / 2)
    );

    solidColorBrush->SetColor(TEXT_COLOR);
    renderTarget->DrawTextLayout(origin, m_pTextLayout, solidColorBrush);

    renderTarget->PopLayer();

}

DOUBLE KeyStrokeItem::width() const
{
    // See draw() for how the box is drawn
    // Because the border is drawn half outside the filled rect, we have the 3x
    // factor. (Otherwise there are antialising issues where edges of border
    // and filled rect meet)
    // BOX_MARGIN is treated as a right margin.
    return (m_textWidth + 2 * BOX_PADDING + 3 * BOX_BORDER + BOX_MARGIN);
}


bool KeyStrokeItem::obscurable() const
{
    // A bit of a kludge but it works for now
    // TODO: add storyboard event handlers 
    UI_ANIMATION_STORYBOARD_STATUS status;
    throw_if_fail(m_pStoryboardOutro->GetStatus(&status));

    return (status == UI_ANIMATION_STORYBOARD_PLAYING);
}


bool KeyStrokeItem::isDone() const
{
    // A bit of a kludge but it works for now
    // TODO: add storyboard event handlers 
    double val;
    throw_if_fail(m_pAnimationVariableAlpha->GetValue(&val));

    return (val == 0);
}

// Return a precedence based on how far left we want to show this key
// Lower precedence = farther left.
// Our order is Win+Ctrl+Alt+Shift+(other key)
UINT KeyStrokeItem::displayPrecedence() const
{
    switch (m_vkCode)
    {
    case(VK_LWIN): case(VK_RWIN):
        return 10;
    case(VK_CONTROL): case(VK_LCONTROL): case(VK_RCONTROL):
        return 20;
    case(VK_MENU): case(VK_LMENU): case(VK_RMENU):
        return 30;
    case(VK_SHIFT): case(VK_LSHIFT): case(VK_RSHIFT):
        return 40;
    default:
        return 50;
    }
}

// Used for sorting keystrokeitems on the screen
bool operator<(KeyStrokeItem a, KeyStrokeItem b)
{
    return a.displayPrecedence() < b.displayPrecedence();
}


HRESULT getKeyString(UINT vkCode, UINT scanCode, LPTSTR dest, int cchDest)
{
    HRESULT hr = S_OK;
    wchar_t * specialName = NULL;

    // Get the string to show based on the key pressed and locale

    // Special case 1
    // PAUSE - the API gives us scan code 0x0
    if (vkCode == VK_PAUSE) {
        scanCode = 0x45;
    }
    // Special case 2
    // The "Microsoft Natural keyboard" keys - unnamed by GetKeyNameText
    // TODO: remove hardcoded strings here
    else if (vkCode == VK_LWIN || vkCode == VK_RWIN) {
        specialName = L"Win";
    }
    else if (vkCode == VK_APPS) {
        specialName = L"Apps";
    }
    // Special case 3
    // GetKeyNameText simply doesn't know about some special keys
    // For example, VK_VOLUME_UP is "B" even with the extended bit.
    // We simply won't handle these media keys
    else if ((vkCode >= 0xA6 && vkCode <= 0xB9) || (vkCode >= 0xE3)) {
        hr = E_FAIL;
    }


    if (specialName && SUCCEEDED(hr)) {
        // Simply copy our custom hardcoded name and return
        hr = StringCchCopy(dest, cchDest, specialName);
    }
    else if (SUCCEEDED(hr)) {
        // We can use GetKeyNameText. Prepare its argument.
        union KeyFlags {
            LONG lParam;
            struct {
                unsigned rc : 16;
                unsigned scanCode : 8;
                unsigned extended : 1;
                unsigned donotcare : 1;
            };
        };

        KeyFlags kf = {};
        kf.scanCode = scanCode;


        // Special Case 4: "Typematic" key API limitations
        // For example, VK_HOME and VK_NUMPAD7 share a scan code but are 
        // distinguished by additional shift/unshift scan codes (see scan code spec)
        // Although the sytem gives us the correct VK_CODE, MapVirtualKey doesn't
        // seem to account for this. 
        // (These keys are all "Typematic" and have special notes in the
        // Keyboard Scan Code Specification)
        switch (vkCode)
        {
        case VK_INSERT: case VK_DELETE: case VK_LEFT: case VK_HOME:
        case VK_END: case VK_UP: case VK_DOWN:case VK_PRIOR:
        case VK_NEXT: case VK_RIGHT: case VK_DIVIDE:
        case VK_NUMLOCK: case VK_SNAPSHOT:
            kf.extended = 1;
            break;
        default:
            kf.extended = 0;
        }

        // Do not differentiate left/right shift, etc.
        // Possible TODO: this design may change
        kf.donotcare = 1;

        int len = GetKeyNameText(kf.lParam, dest, cchDest);
        if (!len) {
            hr = E_FAIL;
        }
    }
    // TODO: results of GetKeyNameText are not always uppercase/sensible for non-US keyboards

    // Don't assert success. We expect a return value of E_FAIL for certain
    // keys we should ignore (e.g. volume keys, laptop Fn key).
    // The destination string will be untouched in this case.
    return hr;
}