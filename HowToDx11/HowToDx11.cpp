// HowToDx11.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Dx11Engine.h"

int main()
{
	const wchar_t wndClass[] = L"WndClass";
	const wchar_t wndTitile[] = L"How to start Directx 11";

	WNDCLASSEXW wcx  {0};
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

	MSG msg;
	BOOL fGotMessage;
	while ((fGotMessage = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0 && fGotMessage != -1)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam
}

