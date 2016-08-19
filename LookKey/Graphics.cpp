#include "stdafx.h"
#include "Graphics.h"
#include "LookKey.h"


GraphicsManager::GraphicsManager()
{
    // Create the objects. These can be reused everywhere.
    initAnimationObjects();
    initDirectWriteObjects();
    initGraphicsObjects();
}

void GraphicsManager::initAnimationObjects()
{
    if (!m_pAnimationManager)
    {
        throw_if_fail(CoCreateInstance(
            CLSID_UIAnimationManager,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_pAnimationManager)
        ));
    }
    if (!m_pAnimationTimer)
    {
        throw_if_fail(CoCreateInstance(
            CLSID_UIAnimationTimer,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_pAnimationTimer)
        ));
    }
    if (!m_pTransitionLibrary)
    {
        throw_if_fail(CoCreateInstance(
            CLSID_UIAnimationTransitionLibrary,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_pTransitionLibrary)
        ));
    }
}

void GraphicsManager::initGraphicsObjects()
{
    if (!m_pD2DFactory) {
        D2D1_FACTORY_OPTIONS options;
#ifdef _DEBUG
        options = { D2D1_DEBUG_LEVEL_INFORMATION };
#else
        options = { D2D1_DEBUG_LEVEL_NONE };
#endif
        throw_if_fail(
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, &m_pD2DFactory)
        );
    }

    if (!m_pWICImagingFactory) {
        throw_if_fail(
            m_pWICImagingFactory.CoCreateInstance(CLSID_WICImagingFactory)
        );
    }
}

void GraphicsManager::initDirectWriteObjects()
{
    if (!m_pDWriteFactory)
    {
        throw_if_fail(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
        ));
    }

    if (!m_pDWriteTextFormat) {
        throw_if_fail(m_pDWriteFactory->CreateTextFormat(
            FONT_FAMILY,
            NULL, // Use system font collection
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            FONT_SIZE,
            L"en-us",
            &m_pDWriteTextFormat
        ));

        // Center horizontally and vertically
        throw_if_fail(m_pDWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
        throw_if_fail(m_pDWriteTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
    }
}

void GraphicsManager::isAnimationManagerBusy(bool* isBusy)
{
    UI_ANIMATION_MANAGER_STATUS status = UI_ANIMATION_MANAGER_IDLE;
    throw_if_fail(m_pAnimationManager->GetStatus(&status));

    *isBusy = (status == UI_ANIMATION_MANAGER_BUSY);
}


void GraphicsManager::updateAnimationManagerTime()
{
    UI_ANIMATION_SECONDS secondsNow = 0;
    throw_if_fail(m_pAnimationTimer->GetTime(&secondsNow));
    throw_if_fail(m_pAnimationManager->Update(secondsNow));
}

void GraphicsManager::getAnimationManager(IUIAnimationManager **out)
{

    throw_if_fail(m_pAnimationManager.CopyTo(out));
}

void GraphicsManager::getAnimationTimer(IUIAnimationTimer **out)
{
    throw_if_fail(m_pAnimationTimer.CopyTo(out));
}

void GraphicsManager::getAnimationTransitionLibrary(IUIAnimationTransitionLibrary **out)
{
    throw_if_fail(m_pTransitionLibrary.CopyTo(out));
}

void GraphicsManager::getD2DFactory(ID2D1Factory ** out)
{
    throw_if_fail(m_pD2DFactory.CopyTo(out));
}

void GraphicsManager::getWICImagingFactory(IWICImagingFactory ** out)
{
    throw_if_fail(m_pWICImagingFactory.CopyTo(out));
}

void GraphicsManager::getDWriteFactory(IDWriteFactory **out)
{
    throw_if_fail(m_pDWriteFactory.CopyTo(out));
}

void GraphicsManager::getDWriteTextFormat(IDWriteTextFormat **out)
{
    throw_if_fail(m_pDWriteTextFormat.CopyTo(out));
}


