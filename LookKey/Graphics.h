#pragma once

const LPCWSTR FONT_FAMILY = L"Segoe UI";
const FLOAT FONT_SIZE = 20.0f;

class GraphicsManager
{
public:
    GraphicsManager();

    void updateAnimationManagerTime();
    void isAnimationManagerBusy(bool* isBusy);

    void getAnimationManager(IUIAnimationManager **out);
    void getAnimationTimer(IUIAnimationTimer **out);
    void getAnimationTransitionLibrary(IUIAnimationTransitionLibrary **out);
    void getD2DFactory(ID2D1Factory **out);
    void getWICImagingFactory(IWICImagingFactory **out);
    void getDWriteFactory(IDWriteFactory **out);
    void getDWriteTextFormat(IDWriteTextFormat **out);


private:
    void initAnimationObjects();
    void initGraphicsObjects();
    void initDirectWriteObjects();


    CComPtr<IUIAnimationManager> m_pAnimationManager;
    CComPtr<IUIAnimationTimer> m_pAnimationTimer;
    CComPtr<IUIAnimationTransitionLibrary> m_pTransitionLibrary;

    CComPtr<ID2D1Factory> m_pD2DFactory;
    CComPtr<IWICImagingFactory> m_pWICImagingFactory;

    CComPtr<IDWriteFactory> m_pDWriteFactory;
    CComPtr<IDWriteTextFormat> m_pDWriteTextFormat;


};
