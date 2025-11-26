/*============================================================================================================

    RenderTargetManagerクラス [direct3d_RTV.h]
    レンダーターゲットビュー（RTV）と深度ステンシルビュー（DSV）を管理するシングルトンクラス。
    スワップチェーンのバックバッファのRTV作成、クリア、バインド、ビューポート設定を行う。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#ifndef DIRECT3D_RENDERTARGETMANAGER_H
#define DIRECT3D_RENDERTARGETMANAGER_H

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>


// RenderTarget (RTV + optional DSV) 管理
class RenderTargetManager
{
public:
	static RenderTargetManager& Instance();
	~RenderTargetManager();

	bool Initialize(ID3D11Device* device, IDXGISwapChain* swapChain); // スワップチェーンバックバッファ RTV 作成
	void Shutdown();

	// クリア & OM バインド
	void BindAndClear(ID3D11DeviceContext* ctx, float r = 0.5f, float g = 0.5f, float b = 0.5f, float a = 1.0f, bool clearDepth = true);

	// ビューポート設定
	void SetViewport(ID3D11DeviceContext* ctx, UINT index = 0);
	UINT GetWidth() const { return m_BackBufferDesc.Width; }
	UINT GetHeight() const { return m_BackBufferDesc.Height; }

	ID3D11RenderTargetView* GetBackBufferRTV() const { return m_BackBufferRTV.Get(); }
	ID3D11DepthStencilView* GetDepthStencilView() const { return m_DepthStencilView.Get(); }

	RenderTargetManager(const RenderTargetManager&) = delete;
	RenderTargetManager& operator=(const RenderTargetManager&) = delete;
	RenderTargetManager(RenderTargetManager&&) = delete;
	RenderTargetManager& operator=(RenderTargetManager&&) = delete;

private:

	RenderTargetManager() = default;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_BackBufferRTV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_DepthStencil;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
	D3D11_TEXTURE2D_DESC m_BackBufferDesc{};
	std::vector<D3D11_VIEWPORT> m_Viewports{};
};
#endif
