//Dx11L01.cpp: 定义应用程序的入口点。
//

#include "pch.h"
#include "Dx11System.h"

const WORD StringBufferLength = 100;

// 全局变量:
WCHAR szTitle[StringBufferLength] = L"DirectX 11 Lesson 01: Setup Development environment";                  // 标题栏文本

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//auto dxSystem = std::make_unique<Dx11System>(hInstance, lpCmdLine);
	Dx11System dxSystem(hInstance, lpCmdLine);

	if (dxSystem.Initialize(szTitle)) {
		dxSystem.Run();
	}
	dxSystem.Destroy();
	//delete dxSystem;
	//dxSystem = nullptr;

	return 0;
}