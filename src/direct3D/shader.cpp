/*==============================================================================

   シェーダー [shader.cpp]
														 Author : Youhei Sato
														 Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3D/direct3d.h"
#include <iostream>
#include <d3dcompiler.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;


static ID3D11VertexShader* g_pVertexShader = nullptr;
static ID3D11InputLayout* g_pInputLayout = nullptr;

static ID3D11Buffer* g_pVSConstantBuffer0 = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer1 = nullptr;

static ID3D11PixelShader* g_pPixelShader = nullptr;
static ID3D11SamplerState* g_pSamplerState = nullptr;
static ID3D11Buffer* g_pPSConstantBuffer = nullptr;

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;


bool Shader_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
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
		hr = D3DReadFileToBlob(L"shaders//cso//VertexShader2D.cso", &vsBlob);

		if (FAILED(hr)) {
			MessageBox(nullptr, "頂点シェーダーの読み込みに失敗しました", "エラー", MB_OK);
			return false;
		}

		hr = g_pDevice->CreateVertexShader(
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			nullptr,
			&g_pVertexShader
		);

		if (FAILED(hr)) {
			std::cout << "Shader_Initialize() : 頂点シェーダーの作成に失敗しました" << std::endl;
			return false;
		}

		// 頂点レイアウトの定義
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION",0, DXGI_FORMAT_R32G32B32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
			{ "COLOR",0, DXGI_FORMAT_R32G32B32A32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
			{ "TEXCOORD",0, DXGI_FORMAT_R32G32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
		};

		UINT num_elements = ARRAYSIZE(layout); // 配列の要素数を取得

		// 頂点レイアウトの作成（blobを使って作成）
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


	//事前コンパイル済みピクセルシェーダーの読み込み（D3DReadFileToBlobを使用）
	{
		ComPtr<ID3DBlob> psBlob;
		hr = D3DReadFileToBlob(L"shaders//cso//PixelShader2D.cso", &psBlob);

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
	//buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // バインドフラグ

	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer);


	// サンプラーステート設定
	D3D11_SAMPLER_DESC sampler_desc{};
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
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

void Shader_Finalize()
{
	SAFE_RELEASE(g_pVertexShader);
	SAFE_RELEASE(g_pInputLayout);

	SAFE_RELEASE(g_pVSConstantBuffer0);
	SAFE_RELEASE(g_pVSConstantBuffer1);

	SAFE_RELEASE(g_pPixelShader);
	SAFE_RELEASE(g_pSamplerState);
	SAFE_RELEASE(g_pPSConstantBuffer);
}

void Shader_SetProjectionMatrix(const DirectX::XMMATRIX& matrix)
{
	// 定数バッファ格納用行列の構造体を定義
	XMFLOAT4X4 transpose;

	// 行列を転置して定数バッファ格納用行列に変換
	XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));

	// 定数バッファに行列をセット
	g_pContext->UpdateSubresource(g_pVSConstantBuffer0,0, nullptr, &transpose,0,0);
}

void Shader_SetWorldMatrix(const DirectX::XMMATRIX& matrix)
{
	// 定数バッファ格納用行列の構造体を定義
	XMFLOAT4X4 transpose;

	// 行列を転置して定数バッファ格納用行列に変換
	XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));

	// 定数バッファに行列をセット
	g_pContext->UpdateSubresource(g_pVSConstantBuffer1,0, nullptr, &transpose,0,0);
}


void Shader_SetColor(const XMFLOAT4& color)
{
	// 定数バッファに行列をセット
	g_pContext->UpdateSubresource(g_pPSConstantBuffer,0, nullptr, &color,0,0);
}

void Shader_Begin()
{
	// 頂点シェーダーとピクセルシェーダーを描画パイプラインに設定
	g_pContext->VSSetShader(g_pVertexShader, nullptr,0);
	g_pContext->PSSetShader(g_pPixelShader, nullptr,0);

	// 頂点レイアウトを描画パイプラインに設定
	g_pContext->IASetInputLayout(g_pInputLayout);

	// 定数バッファを描画パイプラインに設定
	g_pContext->VSSetConstantBuffers(0,1, &g_pVSConstantBuffer0);
	g_pContext->VSSetConstantBuffers(1,1, &g_pVSConstantBuffer1);
	g_pContext->PSSetConstantBuffers(0,1, &g_pPSConstantBuffer);
	

	//サンプラーステートを描画パイプラインに設定
	g_pContext->PSSetSamplers(0,1, &g_pSamplerState);
}
