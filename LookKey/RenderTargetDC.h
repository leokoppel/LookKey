#pragma once

// RenderTargetDC class modified from from Kenny Kerr's sample in "Layered Windows with Direct2D"
class RenderTargetDC {
    ID2D1GdiInteropRenderTarget* m_renderTarget;
    HDC m_dc;

public:
    RenderTargetDC(ID2D1GdiInteropRenderTarget* renderTarget) :
        m_renderTarget(renderTarget),
        m_dc(0) {

        throw_if_fail(m_renderTarget->GetDC(
            D2D1_DC_INITIALIZE_MODE_COPY,
            &m_dc));
    }

    ~RenderTargetDC() {
        RECT rect = {};
        m_renderTarget->ReleaseDC(&rect);
    }

    operator HDC() const {
        return m_dc;
    }
};