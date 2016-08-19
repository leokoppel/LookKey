#pragma once

#include "Graphics.h"

// Application constants
// TODO: move

const bool BEHAVIOUR_REUSE = 0;
const bool BEHAVIOUR_MOVE_FOR_MODIFIERS = 0;



// KeyWindow border in edit mode
const int BORDER_WIDTH = 1;
const int BORDER_HITTEST_WIDTH = 10;
const int BORDER_XPADDING_L = 0;
const int BORDER_XPADDING_R = 0;
const int BORDER_YPADDING = 0;
const int RIGHT_GRADIENT_WIDTH = 150;
const D2D1_COLOR_F EDIT_BORDER_COLOR = D2D1::ColorF(D2D1::ColorF::DarkBlue, 1);
const D2D1_COLOR_F EDIT_INNER_BORDER_COLOR = D2D1::ColorF(D2D1::ColorF::LightSlateGray, 0.9f);
const D2D1_COLOR_F EDIT_FILL_COLOR = D2D1::ColorF(D2D1::ColorF::White, 0.9f);

// Durations in seconds, dimensions in DIPs
const INT MAX_KEY_NAME_LEN = 16;
const DOUBLE BASE_X = BORDER_HITTEST_WIDTH;
const DOUBLE BASE_Y = 50;
const DOUBLE INITIAL_ALPHA = 1;
const DOUBLE INTRO_Y_DIP = 8;
const DOUBLE OUTRO_Y_RISE = 50;
const UI_ANIMATION_SECONDS INTRO_DURATION = 0.04;
const UI_ANIMATION_SECONDS KEYUP_DURATION = INTRO_DURATION;
const UI_ANIMATION_SECONDS HOLD_DURATION = 1;
const UI_ANIMATION_SECONDS OUTRO_DURATION = 0.3;

const DOUBLE BOX_HEIGHT = 24.0;
const DOUBLE BOX_PADDING = 8.0;
const FLOAT BOX_BORDER = 0;
const FLOAT BOX_CORNER_RADIUS = 0;
const DOUBLE BOX_MARGIN = 5.0;

const D2D1_COLOR_F TEXT_COLOR = D2D1::ColorF(1, 1, 1, 1.0f);
const D2D1_COLOR_F BOX_COLOR = D2D1::ColorF(0, 0, 0, 1.0f);
const D2D1_COLOR_F BOX_BORDER_COLOR = D2D1::ColorF(0.3f, 0.3f, 0.3f, 1.0f);
const DOUBLE BOX_ALPHA = 1;

const UI_ANIMATION_SECONDS X_MOVE_DURATION = 0.1;
const DOUBLE X_MOVE_ACCEL = 0.5;
const DOUBLE X_MOVE_DECCEL = 0.5;



class KeyStrokeItem
{
public:
    KeyStrokeItem(UINT vkCode, UINT scanCode, GraphicsManager * graphicsManager);
    ~KeyStrokeItem();
    void setX(DOUBLE x);
    void setInstantX(DOUBLE x);

    void keyDown();
    void keyUp();
    void startOutro();


    void draw(ID2D1RenderTarget * renderTarget, ID2D1Layer * layer, ID2D1SolidColorBrush * solidColorBrush);
    UINT vkCode() const { return m_vkCode; }
    bool isKeyDown() const { return m_keyDown; }
    bool obscurable() const;
    bool isDone() const;
    double width() const;
    double targetX() const { return m_targetX; }
    friend bool operator<(KeyStrokeItem a, KeyStrokeItem b);


private:
    void initStoryboards();
    void initText(GraphicsManager * graphicsManager);
    UINT displayPrecedence() const;

    // State variables
    double m_targetX = 0;
    bool m_set_X = 0;
    UINT m_vkCode = 0;
    UINT m_scanCode = 0;
    bool m_keyDown = true;
    WCHAR m_str[MAX_KEY_NAME_LEN] = L"";
    FLOAT m_textWidth = 0;

    // Animation pointers
    CComPtr<IUIAnimationManager> m_pAnimationManager;
    CComPtr<IUIAnimationTimer> m_pAnimationTimer;
    CComPtr<IUIAnimationTransitionLibrary> m_pTransitionLibrary;

    // Animation variables
    CComPtr<IUIAnimationVariable> m_pAnimationVariableX;
    CComPtr<IUIAnimationVariable> m_pAnimationVariableY;
    CComPtr<IUIAnimationVariable> m_pAnimationVariableAlpha;
    CComPtr<IUIAnimationStoryboard> m_pStoryboardIntro;
    CComPtr<IUIAnimationStoryboard> m_pStoryboardKeyUp;
    CComPtr<IUIAnimationStoryboard> m_pStoryboardOutro;


    CComPtr<IDWriteTextLayout> m_pTextLayout;
};

static HRESULT getKeyString(UINT vkCode, UINT scanCode, LPTSTR dest, int cchDest);


class KeyStrokeItemConstructionException : public std::exception {};
