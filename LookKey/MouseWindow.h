#pragma once
#include "LayeredWindow.h"
#include "MouseItem.h"

class MouseWindow :
    public LayeredWindow
{
public:
    MouseWindow(GraphicsManager * graphicsManager, ConfigManager * config);
    ~MouseWindow();
    void setEnabled(bool enabled);

private:
    // Derived methods
    LRESULT CALLBACK HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void drawClientArea();
    void periodicUpdate();

    // Specific methods
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
    void initInputHook();
    void unhookInputHook();
    void handleMouseButton(POINT pt, POINT direction, UINT msg);
    void updatePos();

    static MouseWindow * s_pThis; // Used for input hook callback

    std::vector<std::unique_ptr<MouseItem>> m_mouseItems;
    POINT m_mousePos;
    bool m_LButtonDown = 0;
    bool m_RButtonDown = 0;
    bool m_MButtonDown = 0;
    HHOOK m_inputHook = 0;

};

