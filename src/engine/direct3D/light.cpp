#include "light.h"
#include "direct3d.h"

using namespace DirectX;

static ID3D11Buffer* g_pPSConstantBuffer0;
static ID3D11Buffer* g_pPSConstantBuffer1;

void Light_Init()
{
	auto pDevice = Direct3D_GetDevice();

	// ピクセルシェーダーのライティング用定数バッファの作成
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // バインドフラグ

	buffer_desc.ByteWidth = sizeof(XMFLOAT4); // バッファのサイズ
	pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer0);

	buffer_desc.ByteWidth = sizeof(Diffuse); // バッファのサイズ
	pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer1);
}

void Light_UnInit()
{
	SAFE_RELEASE(g_pPSConstantBuffer0);
	SAFE_RELEASE(g_pPSConstantBuffer1);
}

void Light_SetAmbient(const XMFLOAT4& color)
{
	// 定数バッファにambienにカラーをセット
	Direct3D_GetDeviceContext()->UpdateSubresource(g_pPSConstantBuffer0, 0, nullptr, &color, 0, 0);
	Direct3D_GetDeviceContext()->PSSetConstantBuffers(0, 1, &g_pPSConstantBuffer0);
}

void Light_SetDiffuse(const XMFLOAT4& color, const XMFLOAT3& direction)
{
	Diffuse light{ color, {direction.x, direction.y, direction.z, 1.0f} };

	// 定数バッファにdiffuseセット
	Direct3D_GetDeviceContext()->UpdateSubresource(g_pPSConstantBuffer1, 0, nullptr, &light, 0, 0);
	Direct3D_GetDeviceContext()->PSSetConstantBuffers(1, 1, &g_pPSConstantBuffer1);
}

void Light_SetDiffuse(const Diffuse& light)
{
	// 定数バッファにdiffuseセット
	Direct3D_GetDeviceContext()->UpdateSubresource(g_pPSConstantBuffer1, 0, nullptr, &light, 0, 0);
	Direct3D_GetDeviceContext()->PSSetConstantBuffers(1, 1, &g_pPSConstantBuffer1);
}
