#include "stdafx.h"
#include "Debug.h"


namespace Debug
{
    // Initialize global string stream
    DebugStream out;


    void FpsCounter::reset()
    {
        m_frameCount = 0;
        m_startTicks = GetTickCount64();
    }
    void FpsCounter::countFrame()
    {
        m_frameCount++;
    }

    // Return fps since last reset()
    double FpsCounter::getFps()
    {
        ULONGLONG currentTicks = GetTickCount64();

        ULONGLONG totalTimeMs = currentTicks - m_startTicks;
        double fps = -1;
        if (totalTimeMs > 0) {
            fps = m_frameCount / (totalTimeMs / 1000.0);
        }
        return fps;
    }


}