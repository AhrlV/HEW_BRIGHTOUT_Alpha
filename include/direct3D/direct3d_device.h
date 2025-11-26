/*============================================================================================================

    GraphicsDeviceクラス [direct3d_device.h]
    Direct3D11デバイスとコンテキストを管理するシングルトンクラス。
    デバイスの初期化、スワップチェーンの管理、画面の更新を行う。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#ifndef DIRECT3D_DEVICE_H
#define DIRECT3D_DEVICE_H


#include <dxgi.h>
#include <wrl/client.h>
#include <Windows.h>
#include <d3d11.h>

class GraphicsDevice
{
public:
	// シングルトン取得
	static GraphicsDevice& Instance();

	~GraphicsDevice();

	void Initialize(HWND hWnd, UINT vsyncInterval = 1);
	void Shutdown();

	ID3D11Device* GetDevice() const { return m_Device.Get(); }
	ID3D11DeviceContext* GetContext() const { return m_Context.Get(); }
	IDXGISwapChain* GetSwapChain() const { return m_SwapChain.Get(); }
	UINT GetVsyncInterval() const { return m_VsyncInterval; }

	void Present();

	// コピー/ムーブ禁止
	GraphicsDevice(const GraphicsDevice&) = delete;
	GraphicsDevice& operator=(const GraphicsDevice&) = delete;
	GraphicsDevice(GraphicsDevice&&) = delete;
	GraphicsDevice& operator=(GraphicsDevice&&) = delete;

private:

	GraphicsDevice() = default;

	Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_Context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
	UINT m_VsyncInterval = 1;
};

#endif // DIRECT3D_DEVICE_H
