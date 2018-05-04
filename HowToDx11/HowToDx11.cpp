// HowToDx11.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Dx11Engine.h"

LRESULT CALLBACK MainWndProc(
	_In_ HWND   hwnd,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam) {

	switch (uMsg) {
	case WM_KEYDOWN:
		if (VK_ESCAPE == wParam) {
			PostQuitMessage(0);
		}
		break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_QUIT:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
};


int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	const wchar_t wndClass[] = L"WndClass";
	const wchar_t wndTitile[] = L"How to start Directx 11";

	WNDCLASSEXW wcx{ 0 };
	wcx.cbSize = sizeof(wcx);
	wcx.lpszClassName = wndClass;
	wcx.hInstance = GetModuleHandleW(NULL);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = MainWndProc;

	RegisterClassExW(&wcx);

	HWND hwnd = CreateWindowExW(
		WS_EX_APPWINDOW,
		wcx.lpszClassName,        // name of window class 
		wndTitile,            // title-bar string 
		WS_OVERLAPPEDWINDOW, // top-level window 
		CW_USEDEFAULT,       // default horizontal position 
		CW_USEDEFAULT,       // default vertical position 
		CW_USEDEFAULT,       // default width 
		CW_USEDEFAULT,       // default height 
		(HWND)NULL,         // no owner window 
		(HMENU)NULL,        // use class menu 
		wcx.hInstance,           // handle to application instance 
		(LPVOID)NULL);      // no window-creation data 

	if (!hwnd)
		return FALSE;

	Dx11Engine eng;
	eng.Initialize(hwnd);

	// Show the window and send a WM_PAINT message to the window 
	// procedure. 

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	MSG msg{ 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			eng.Render();
		}
	}

	return (int)msg.wParam;
}

