/*==============================================================================

   シェーダー [shader3d.cpp]
														 Author : Youhei Sato
														 Date   :2025/10/15
--------------------------------------------------------------------------------

==============================================================================*/
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3D/direct3d.h"
#include <iostream>
#include "direct3D/shader3d.h"
#include <d3dcompiler.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

static ID3D11VertexShader* g_pVertexShader = nullptr;
static ID3D11InputLayout* g_pInputLayout = nullptr;

static ID3D11Buffer* g_pVSConstantBuffer0 = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer1 = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer2 = nullptr;

static ID3D11Buffer* g_pPSConstantBuffer2 = nullptr;

static ID3D11PixelShader* g_pPixelShader = nullptr;
static ID3D11SamplerState* g_pSamplerState = nullptr;

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;


bool Shader3d_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	HRESULT hr; // 戻り値格納用

	// デバイスとデバイスコンテキストのチェック
	if (!pDevice || !pContext) {
		std::cout << "Shader_Initialize() : 与えられたデバイスかコンテキストが不正です" << std::endl;
		return false;
	}

	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	//事前コンパイル済み頂点シェーダーの読み込み（D3DReadFileToBlobを使用）
	{
		ComPtr<ID3DBlob> vsBlob;
		hr = D3DReadFileToBlob(L"shaders//cso//VertexShader3D.cso", &vsBlob);

		if (FAILED(hr)) {
			MessageBox(nullptr, "頂点シェーダーの読み込みに失敗しました\n\nshader_vertex_2d.cso", "エラー", MB_OK);
			return false;
		}

		hr = g_pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_pVertexShader);

		if (FAILED(hr)) {
			std::cout << "Shader_Initialize() : 頂点シェーダーの作成に失敗しました" << std::endl;
			return false;
		}

		// 頂点レイアウトの定義
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION",0, DXGI_FORMAT_R32G32B32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
			{ "COLOR",0, DXGI_FORMAT_R32G32B32A32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
			{ "NORMAL",0, DXGI_FORMAT_R32G32B32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
			{ "TEXCOORD",0, DXGI_FORMAT_R32G32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
			{ "TEXUSE",0, DXGI_FORMAT_R32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
		};

		UINT num_elements = ARRAYSIZE(layout); // 配列の要素数を取得

		// 頂点レイアウトの作成
		hr = g_pDevice->CreateInputLayout(layout, num_elements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_pInputLayout);

		if (FAILED(hr)) {
			MessageBox(nullptr, "Shader_Initialize() : 頂点レイアウトの作成に失敗しました", "エラー", MB_OK);
			return false;
		}
	}

	// 頂点シェーダー用定数バッファの作成
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(XMFLOAT4X4); // バッファのサイズ
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // バインドフラグ

	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer0);
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer1);
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer2);

	//事前コンパイル済みピクセルシェーダーの読み込み（D3DReadFileToBlobを使用）
	{
		ComPtr<ID3DBlob> psBlob;
		hr = D3DReadFileToBlob(L"shaders//cso//PixelShader3D.cso", &psBlob);

		if (FAILED(hr)) {
			MessageBox(nullptr, "ピクセルシェーダーの読み込みに失敗しました\n\nshader_pixel_2d.cso", "エラー", MB_OK);
			return false;
		}

		hr = g_pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_pPixelShader);

		if (FAILED(hr)) {
			std::cout << "Shader_Initialize() : ピクセルシェーダーの作成に失敗しました" << std::endl;
			return false;
		}
	}

	// ピクセルシェーダー用定数バッファの作成
	buffer_desc.ByteWidth = sizeof(XMFLOAT4); // バッファのサイズ

	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer2);


	// サンプラーステート設定
	D3D11_SAMPLER_DESC sampler_desc{};
	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	sampler_desc.BorderColor[0] =0.0f;
	sampler_desc.BorderColor[1] =0.0f;
	sampler_desc.BorderColor[2] =0.0f;
	sampler_desc.BorderColor[3] =1.0f;

	sampler_desc.MipLODBias =0;
	sampler_desc.MaxAnisotropy =16;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler_desc.MinLOD =0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	g_pDevice->CreateSamplerState(&sampler_desc, &g_pSamplerState);


	return true;
}

void Shader3d_Finalize()
{
	SAFE_RELEASE(g_pVertexShader);
	SAFE_RELEASE(g_pInputLayout);

	SAFE_RELEASE(g_pVSConstantBuffer0);
	SAFE_RELEASE(g_pVSConstantBuffer1);
	SAFE_RELEASE(g_pVSConstantBuffer2);

	SAFE_RELEASE(g_pPSConstantBuffer2);

	SAFE_RELEASE(g_pPixelShader);
	SAFE_RELEASE(g_pSamplerState);
}


void Shader3d_SetWorldMatrix(const DirectX::XMMATRIX& matrix)
{
	// 定数バッファ格納用行列の構造体を定義
	XMFLOAT4X4 transpose;

	// 行列を転置して定数バッファ格納用行列に変換
	XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));

	// 定数バッファに行列をセット
	g_pContext->UpdateSubresource(g_pVSConstantBuffer0,0, nullptr, &transpose,0,0);
}

void Shader3d_SetViewMatrix(const DirectX::XMMATRIX& matrix)
{
	// 定数バッファ格納用行列の構造体を定義
	XMFLOAT4X4 transpose;

	// 行列を転置して定数バッファ格納用行列に変換
	XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));

	// 定数バッファに行列をセット
	g_pContext->UpdateSubresource(g_pVSConstantBuffer1,0, nullptr, &transpose,0,0);
}

void Shader3d_SetProjectionMatrix(const DirectX::XMMATRIX& matrix)
{
	// 定数バッファ格納用行列の構造体を定義
	XMFLOAT4X4 transpose;

	// 行列を転置して定数バッファ格納用行列に変換
	XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));

	// 定数バッファに行列をセット
	g_pContext->UpdateSubresource(g_pVSConstantBuffer2,0, nullptr, &transpose,0,0);
}

void Shader3d_SetMaterialDiffuse(const DirectX::XMFLOAT4 material)
{
	// 定数バッファに行列をセット
	g_pContext->UpdateSubresource(g_pPSConstantBuffer2,0, nullptr, &material,0,0);
}


void Shader3d_Begin()
{
	// 頂点シェーダーとピクセルシェーダーを描画パイプラインに設定
	g_pContext->VSSetShader(g_pVertexShader, nullptr,0);
	g_pContext->PSSetShader(g_pPixelShader, nullptr,0);

	// 頂点レイアウトを描画パイプラインに設定
	g_pContext->IASetInputLayout(g_pInputLayout);

	// 定数バッファを描画パイプラインに設定
	g_pContext->VSSetConstantBuffers(0,1, &g_pVSConstantBuffer0);
	g_pContext->VSSetConstantBuffers(1,1, &g_pVSConstantBuffer1);
	g_pContext->VSSetConstantBuffers(2,1, &g_pVSConstantBuffer2);
	g_pContext->PSSetConstantBuffers(2,1, &g_pPSConstantBuffer2);

	//サンプラーステートを描画パイプラインに設定
	g_pContext->PSSetSamplers(0,1, &g_pSamplerState);
}
