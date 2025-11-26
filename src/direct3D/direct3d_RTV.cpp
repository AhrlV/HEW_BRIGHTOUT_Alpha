#include "direct3D/direct3d_RTV.h"
#include <stdexcept>

RenderTargetManager& RenderTargetManager::Instance(){ static RenderTargetManager inst; return inst; }

RenderTargetManager::~RenderTargetManager(){ Shutdown(); }

bool RenderTargetManager::Initialize(ID3D11Device* device, IDXGISwapChain* swapChain)
{
	if (!device || !swapChain) throw std::runtime_error("device/swapchain invalid");
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

	if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())))) 
		throw std::runtime_error("backbuffer get failed");
	if (FAILED(device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_BackBufferRTV.GetAddressOf()))) 
		throw std::runtime_error("RTV create failed");

	backBuffer->GetDesc(&m_BackBufferDesc);
	D3D11_TEXTURE2D_DESC depthDesc{}; 
	depthDesc.Width = m_BackBufferDesc.Width; 
	depthDesc.Height = m_BackBufferDesc.Height; 
	depthDesc.MipLevels=1; depthDesc.ArraySize=1; 
	depthDesc.Format=DXGI_FORMAT_D24_UNORM_S8_UINT; 
	depthDesc.SampleDesc.Count=1; 
	depthDesc.SampleDesc.Quality=0; 
	depthDesc.Usage=D3D11_USAGE_DEFAULT; 
	depthDesc.BindFlags=D3D11_BIND_DEPTH_STENCIL;

	if (FAILED(device->CreateTexture2D(&depthDesc, nullptr, m_DepthStencil.GetAddressOf()))) 
		throw std::runtime_error("depth tex create failed");
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{}; 
	dsvDesc.Format = depthDesc.Format; 
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; 
	dsvDesc.Texture2D.MipSlice = 0;

	if (FAILED(device->CreateDepthStencilView(m_DepthStencil.Get(), &dsvDesc, m_DepthStencilView.GetAddressOf()))) 
		throw std::runtime_error("DSV create failed");
	m_Viewports.clear();

	D3D11_VIEWPORT vp{}; 
	vp.TopLeftX=0; 
	vp.TopLeftY=0; 
	vp.Width=(FLOAT)m_BackBufferDesc.Width; 
	vp.Height=(FLOAT)m_BackBufferDesc.Height; 
	vp.MinDepth=0; 
	vp.MaxDepth=1; 
	m_Viewports.push_back(vp);
	return true;
}

void RenderTargetManager::Shutdown(){ m_DepthStencilView.Reset(); m_DepthStencil.Reset(); m_BackBufferRTV.Reset(); }

void RenderTargetManager::BindAndClear(ID3D11DeviceContext* ctx, float r, float g, float b, float a, bool clearDepth){ if(!ctx || !m_BackBufferRTV) return; float color[4]={r,g,b,a}; ctx->ClearRenderTargetView(m_BackBufferRTV.Get(), color); if(clearDepth && m_DepthStencilView) ctx->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH,1.f,0); ctx->OMSetRenderTargets(1,m_BackBufferRTV.GetAddressOf(),m_DepthStencilView.Get()); }

void RenderTargetManager::SetViewport(ID3D11DeviceContext* ctx, UINT index){ if(!ctx || index>=m_Viewports.size()) return; ctx->RSSetViewports(1,&m_Viewports[index]); }
