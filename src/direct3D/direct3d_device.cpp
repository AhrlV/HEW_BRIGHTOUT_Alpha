#include "direct3D/direct3d_device.h"
#include <stdexcept>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

GraphicsDevice& GraphicsDevice::Instance()
{ 
	static GraphicsDevice inst; 
	return inst; 
}


GraphicsDevice::~GraphicsDevice()
{
	Shutdown();
}

void GraphicsDevice::Initialize(HWND hWnd, UINT vsyncInterval)
{
	m_VsyncInterval = vsyncInterval;
	UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	DXGI_SWAP_CHAIN_DESC scDesc{};
	scDesc.BufferCount = 2;
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.OutputWindow = hWnd;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.Windowed = TRUE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL obtained = D3D_FEATURE_LEVEL_11_0;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr, 
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr, 
		flags,
		featureLevels, 
		(UINT)(sizeof(featureLevels) / sizeof(featureLevels[0])),
		D3D11_SDK_VERSION, 
		&scDesc, 
		m_SwapChain.GetAddressOf(), 
		m_Device.GetAddressOf(), 
		&obtained, 
		m_Context.GetAddressOf());

	if (FAILED(hr))
	{
		throw std::runtime_error("SwapChain ì¬Ž¸”s");
	}
}

void GraphicsDevice::Shutdown()
{
	if (m_Context) { m_Context->ClearState(); m_Context->Flush(); }
	m_SwapChain.Reset();
	m_Context.Reset();
	m_Device.Reset();
}

void GraphicsDevice::Present()
{
	if (m_SwapChain) m_SwapChain->Present(m_VsyncInterval, 0);
}
