#pragma once

// BaseWindow class modified from WinApi documentation sample in "Managing Application State"
template <class DERIVED_TYPE>
class BaseWindow
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DERIVED_TYPE *pThis = NULL;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (DERIVED_TYPE*)pCreate->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->m_hwnd = hwnd;
		}
		else
		{
			pThis = (DERIVED_TYPE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}
		if (pThis)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}

	BaseWindow(PCWSTR classname) :
		m_classname(classname),
		m_hwnd(NULL)
	{ }

	BOOL Create(
		PCWSTR lpWindowName,
		DWORD dwStyle,
		DWORD dwExStyle = 0,
		WNDCLASSEX wcex = { 0 },
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,
		int nWidth = CW_USEDEFAULT,
		int nHeight = CW_USEDEFAULT,
		HWND hWndParent = 0,
		HMENU hMenu = 0
	)
	{
		// Add to passed WNDCLASSEX structure

		wcex.cbSize = sizeof(wcex);
		wcex.lpfnWndProc = DERIVED_TYPE::WindowProc;
		wcex.hInstance = GetModuleHandle(NULL);
		wcex.lpszClassName = m_classname;

		RegisterClassExW(&wcex);

		m_hwnd = CreateWindowEx(
			dwExStyle, m_classname, lpWindowName, dwStyle, x, y,
			nWidth, nHeight, hWndParent, hMenu, GetModuleHandle(NULL), this
		);

        return (m_hwnd != 0);
	}

	HWND hWnd() const { return m_hwnd; }


protected:
	virtual LRESULT CALLBACK HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	PCWSTR m_classname;
	HWND m_hwnd;

};