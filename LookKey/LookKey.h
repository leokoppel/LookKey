#pragma once

#include "Resource.h"

inline void throw_if_fail(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw _com_error(hr);
    }
}

inline void throw_if_fail(BOOL b)
{
    if (!b)
    {
        throw _com_error(E_FAIL);
    }
}


inline LONG_PTR EditWindowLong(HWND hWnd, int nIndex, LONG bitmask, bool setOrUnset)
{
    LONG_PTR wl = GetWindowLongPtr(hWnd, nIndex);
    if (setOrUnset) {
        wl |= bitmask;
    }
    else {
        wl &= ~bitmask;
    }
    return SetWindowLongPtr(hWnd, nIndex, wl);
}

// Convenience operator for adding two points
inline D2D1_POINT_2F operator+(const D2D1_POINT_2F& a, const D2D1_POINT_2F& b)
{
    return{ a.x + b.x, a.y + b.y };
}