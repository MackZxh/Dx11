#include "pch.h"
#include "D3D11Engine.h"
#include "DXHelper.h"


D3D11Engine::D3D11Engine()
{
}

D3D11Engine::~D3D11Engine()
{
}

void D3D11Engine::BeginScene(float cr, float cg, float cb, float ca)
{
	// Clear the back buffer.
	float color[4] = { cr, cg, cb, ca };
	m_Context->ClearRenderTargetView(m_RenderTargetView.Get(), color);

	// Clear the depth buffer.
	m_Context->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0F, 0);


	// Now set the rasterizer state.
	m_Context->RSSetState(m_RasterizerState.Get());

	// Create the m_Viewport.
	m_Context->RSSetViewports(1, &m_Viewport);

	// Set the depth stencil state.
	m_Context->OMSetDepthStencilState(m_DepthStencilState.Get(), 1);

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_Context->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());
}

void D3D11Engine::Render()
{
	 // todo:
}

void D3D11Engine::EndScene()
{
	/************************************************************************/
	/* Present the frame.                                                   */
	/************************************************************************/
	HRESULT hr;
	if (m_bVsync) {
		// Lock to screen refresh rate.
		hr = m_SwapChain->Present(1, 0);
	}
	else {
		// Present as fast as possible.
		hr = m_SwapChain->Present(0, 0);
	}
	if (FAILED(hr)) {
		hr = m_Device->GetDeviceRemovedReason();
	}
}

HRESULT D3D11Engine::GetHardwareAdapter(IDXGIFactory4* pFactory, UINT Width, UINT Height, IDXGIAdapter1** ppAdapter, IDXGIOutput** ppOutput, DXGI_MODE_DESC* pModeDesc) {
	*ppAdapter = nullptr;
	HRESULT hr = E_FAIL;
	for (UINT adapterIndex = 0; ; ++adapterIndex) {
		IDXGIAdapter1* pAdapter = nullptr;

		hr = pFactory->EnumAdapters1(adapterIndex, &pAdapter);
		if (DXGI_ERROR_NOT_FOUND == hr) {
			// No more adapters to enumerate.
			break;
		}
		*ppAdapter = pAdapter;

		for (UINT outputIndex = 0; ; ++outputIndex) {
			IDXGIOutput* pOutput;
			hr = pAdapter->EnumOutputs(outputIndex, &pOutput);
			if (DXGI_ERROR_NOT_FOUND == hr) {
				// No more output to enumerate.
				break;
			}
			*ppOutput = pOutput;

			UINT numModes;
			DXGI_MODE_DESC* pModeDesc{ nullptr };
			hr = pOutput->GetDisplayModeList(m_BufferFormat, DXGI_ENUM_MODES_INTERLACED, &numModes, pModeDesc);
			if (FAILED(hr)) {
				break;
			}
			pModeDesc = new DXGI_MODE_DESC[numModes];
			hr = pOutput->GetDisplayModeList(m_BufferFormat, DXGI_ENUM_MODES_INTERLACED, &numModes, pModeDesc);
			if (FAILED(hr)) {
				break;
			}
			for (UINT m = 0; m < numModes; m++) {
				if (pModeDesc[m].Width == Width && pModeDesc[m].Height == Height) {
					*pModeDesc = pModeDesc[m];
					delete[] pModeDesc;
					return S_OK;
				}
			}
			delete[] pModeDesc;
			pOutput->Release();
		}
		pAdapter->Release();
	}

	return hr;
}

bool D3D11Engine::LoadPipeline(HWND hWnd, UINT Width, UINT Height)
{
	OutputDebugStringW(L"LoadPipeline.\n");
	/************************************************************************/
	/*  Initialize the pipeline.                                            */
	/************************************************************************/
	HRESULT hr;
	UINT dxDebugEnable = 0;
#if defined(_DEBUG)
	dxDebugEnable |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	// Create the device.
	ComPtr<IDXGIFactory4> factory;
	hr = CreateDXGIFactory2(dxDebugEnable, IID_PPV_ARGS(&factory));
	ThrowIfFailed(hr);

	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIOutput> adapterOutput;
	DXGI_MODE_DESC modeDesc;
	if (m_Warp) {
		hr = factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		ThrowIfFailed(hr);
	}
	else {
		hr = GetHardwareAdapter(factory.Get(), Width, Height, &adapter, &adapterOutput, &modeDesc);
	}

	DXGI_ADAPTER_DESC adapterDesc;
	hr = adapter->GetDesc(&adapterDesc);
	ThrowIfFailed(hr);

	m_VideoMemory = (int)adapterDesc.DedicatedVideoMemory / 1024 / 1024;
	m_VideoCard = adapterDesc.Description;

	//adapterOutput.Release();
	//adapter->Release();
	//factory->Release();

	// Create the device and swap chain.
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = Width;
	swapChainDesc.BufferDesc.Height = Height;
	swapChainDesc.BufferDesc.Format = m_BufferFormat;
	// Set the refresh rate of the back buffer.
	if (m_bVsync)
	{
		swapChainDesc.BufferDesc.RefreshRate = modeDesc.RefreshRate;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // DXGI_SWAP_EFFECT_FLIP_DISCARD
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = !m_bFullScreen;
	swapChainDesc.Flags = 0;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_SwapChain, &m_Device, NULL, &m_Context);
	if (FAILED(hr))
	{
		OutputDebugStringW(L"Create Directx 11 Device fail.\n");
		return false;
	}

	// Get the pointer to the back buffer.
	ID3D11Texture2D* backBufferPtr;
	hr = m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBufferPtr));
	if (FAILED(hr))
	{
		OutputDebugStringW(L"Get swapchain buffer fail.\n");
		return false;
	}

	// Create the render target view with the back buffer pointer.
	hr = m_Device->CreateRenderTargetView(backBufferPtr, NULL, &m_RenderTargetView);
	if (FAILED(hr))
	{
		OutputDebugStringW(L"CreateRenderTargetView fail.\n");
		return false;
	}

	// Release pointer to the back buffer as we no longer need it.
	SafeRelease(backBufferPtr);

	// Initialize the description of the depth buffer.
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
	// Set up the description of the depth buffer.
	depthBufferDesc.Width = Width;
	depthBufferDesc.Height = Height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = m_DepthStencilFormat;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	ComPtr<ID3D11Texture2D> m_DepthStencilBuffer{ nullptr };

	hr = m_Device->CreateTexture2D(&depthBufferDesc, NULL, &m_DepthStencilBuffer);
	if (FAILED(hr))
	{
		OutputDebugStringW(L"Create DepthStencilBuffer fail.\n");
		return false;
	}

	// Initialize the description of the stencil state.
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	hr = m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);
	if (FAILED(hr))
	{
		OutputDebugStringW(L"CreateDepthStencilState fail.\n");
		return false;
	}

	// Initialize the depth stencil view.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = m_DepthStencilFormat;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	hr = m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &depthStencilViewDesc, &m_DepthStencilView);
	if (FAILED(hr))
	{
		OutputDebugStringW(L"CreateDepthStencilView fail.\n");
		return false;
	}

	// Setup the raster description which will determine how and what polygons will be drawn.
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0F;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0F;

	// Create the rasterizer state from the description we just filled out.
	hr = m_Device->CreateRasterizerState(&rasterDesc, &m_RasterizerState);
	if (FAILED(hr))
	{
		return false;
	}

	// This sample does not support fullscreen transitions.
	hr = factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
	ThrowIfFailed(hr);

	return true;
}

bool D3D11Engine::Initialize(HWND hWnd, int Width, int Height, float ScreenDepth, float ScreenNear, bool vSync, bool fullscreen)
{
	m_bFullScreen = fullscreen;
	m_bVsync = vSync;
	float aspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
	// Setup the viewport for rendering.
	m_Viewport = { 0.0F, 0.0F, static_cast<float>(Width), static_cast<float>(Height),  0.0F, 1.0F };

	// Create the projection matrix for 3D rendering.
	m_matProjection = XMMatrixPerspectiveFovLH(m_FOV, aspectRatio, ScreenNear, ScreenDepth);
	// Initialize the world matrix to the identity matrix.
	m_matWorld = XMMatrixIdentity();
	// Create an orthographic projection matrix for 2D rendering.
	m_matOrtho = XMMatrixOrthographicLH((float)Width, (float)Height, ScreenNear, ScreenDepth);

	if (!LoadPipeline(hWnd, Width, Height)) {
		OutputDebugStringW(L"Pipeline initialize fail.");
		return false;
	}

	return true;
}

void D3D11Engine::Destroy()
{
	// Wait for the GPU to finish.;
	//WaitforPreviousFrame();

	// Close the event handle.
	//CloseHandle(m_FenceEvent);
}

bool D3D11Engine::UpdateFrame()
{
	// Modify the constant, vertex, index buffers, and everything else, as necessary.
	return true;
}