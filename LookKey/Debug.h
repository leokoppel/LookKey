#pragma once

#include <cstdarg>
#include <sstream>


// Debugging utility functions

namespace Debug
{

    // DebugStream is equivalent to wostringstream, but prints to debug console.
    class DebugStream : public std::wostream
    {
        // Derived stream buffer that prints each line to debug console
        // (Write std::endl or flush() the stream to trigger sync)
        class DebugStreamBuf : public std::wstringbuf
        {
        public:
            // Output and reset the buffer
            virtual int sync() {
                OutputDebugString(str().c_str());
                return 0;
            }
        };

        DebugStreamBuf buffer;

    public:
        DebugStream() : std::wostream(&buffer) {}
    };

    extern DebugStream out;

    class FpsCounter
    {
        ULONGLONG  m_startTicks;
        int m_frameCount;

    public:
        FpsCounter() { reset(); }
        void reset();
        void countFrame();
        double getFps();
        int getFrameCount() const { return m_frameCount;  }
    };

}
