#pragma once

#include "KeyStrokeItem.h"
#include "LayeredWindow.h"
#include "Graphics.h"
#include "Config.h"
#include "Debug.h"


// Window dimensions in px
const int CXMINTRACK = 50;
const int CYMINTRACK = 50;
const int CXMAXTRACK = 0;
const int CYMAXTRACK = 0;

class KeyWindow : public LayeredWindow
{
public:
    KeyWindow(GraphicsManager * graphicsManager, ConfigManager * config);
    ~KeyWindow();


    void setEditMode(bool);
    void toggleEditMode();
    void enterEditMode();
    void exitEditMode();
    BOOL inEditMode() const;

    void handleKeyDown(UINT vkCode, UINT scanCode);
    void addNewItem(UINT vkCode, UINT scanCode);
    void handleKeyUp(UINT vkCode, UINT scanCode);

    void setEnabled(bool enabled);

private:
    // Derived methods
    LRESULT CALLBACK HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void drawClientArea();
    float calculateScale() const;
    void periodicUpdate();

    // Specific methods
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    void initInputHook();
    void unhookInputHook();
    std::vector<KeyStrokeItem>::const_iterator findNewPosition(KeyStrokeItem &k);
    void reCalculateLayout();
    void removeStuckItems();
    LRESULT clientHitTest(int cx, int cy);

    static KeyWindow * s_pThis; // Used for input hook callback

    std::vector<KeyStrokeItem> m_keyStrokeItems;
    BOOL m_editMode = 0;
    HHOOK m_inputHook = 0;
    CComPtr<IDWriteTextLayout> m_pTextLayout; // Layout for edit mode text



};
