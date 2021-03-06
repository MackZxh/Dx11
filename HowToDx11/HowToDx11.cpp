// HowToDx11.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Dx11Engine.h"
#include "Dx11Model.h"
#include "Dx11Shader.h"
#include "Dx11Camera.h"

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
	if (!DirectX::XMVerifyCPUSupport()) {
		OutputDebugString(L"DirectXMath not support!\n");
		return -1;
	}

	const wchar_t wndClass[] = L"WndClass";
	const wchar_t wndTitile[] = L"How to start Directx 11";

	WNDCLASSEXW wcx{ 0 };
	wcx.cbSize = sizeof(wcx);
	wcx.lpszClassName = wndClass;
	wcx.hInstance = GetModuleHandleW(NULL);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = MainWndProc;

	RegisterClassExW(&wcx);

	const int width = 800, height = 600;
	RECT rc{ 0, 0, width, height };
	OffsetRect(&rc, (GetSystemMetrics(SM_CXFULLSCREEN) - width) / 2, (GetSystemMetrics(SM_CYFULLSCREEN) - height) / 2);
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hwnd = CreateWindowExW(
		WS_EX_APPWINDOW,
		wcx.lpszClassName,        // name of window class 
		wndTitile,            // title-bar string 
		WS_OVERLAPPEDWINDOW, // top-level window 
		rc.left,       // default horizontal position 
		rc.top,       // default vertical position 
		rc.right - rc.left,       // default width 
		rc.bottom - rc.top,       // default height 
		(HWND)NULL,         // no owner window 
		(HMENU)NULL,        // use class menu 
		wcx.hInstance,           // handle to application instance 
		(LPVOID)NULL);      // no window-creation data 

	if (!hwnd)
		return -1;

	Dx11Engine eng;
	if (FAILED(eng.Initialize(hwnd, width, height))) {
		return -4;
	}

	// Show the window and send a WM_PAINT message to the window 
	// procedure. 

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	MSG msg{ 0 };
	static DWORD previousTime = timeGetTime();

	while (WM_QUIT != msg.message)
	{
		DWORD currentTime = timeGetTime();
		if (PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (SUCCEEDED(eng.UpdateFrame(float(currentTime - previousTime) / 1000.0f))) {

			eng.DrawFrame();

		}
		previousTime = currentTime;
	}
	eng.Destroy();

	return (int)msg.wParam;
}

