// pch.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件:
#include <windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

//// C 运行时头文件
#include <stdlib.h>
//#include <malloc.h>
//#include <memory.h>
//#include <tchar.h>

// C++ 运行时头文件
#include <memory>
#include <string>
#include <iostream>
#include <sstream>
#include <exception>
#include <vector>

// TODO: 在此处引用程序需要的其他头文件
#include <D3d11_4.h>
#include <D3DCompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <dxgi1_4.h>

#include <wrl.h>
#include <shellapi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")


namespace DX
{
	inline void ThrowIfFailed1(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch DirectX API errors
			throw std::exception();
		}
	}

	// Helper class for COM exceptions
	class com_exception : public std::exception
	{
	public:
		com_exception(HRESULT hr) : result(hr) {}

		virtual const char* what() const override
		{
			static char s_str[64] = {};
			sprintf_s(s_str, "Failure with HRESULT of %08X",
				static_cast<unsigned int>(result));
			return s_str;
		}

	private:
		HRESULT result;
	};

	// Helper utility converts D3D API failures into exceptions.
	inline void ThrowIfFailed2(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw com_exception(hr);
		}
	}
}