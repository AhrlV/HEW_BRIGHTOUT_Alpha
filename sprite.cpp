/*==============================================================================

   スプライト描画 [sprite.cpp]
														 Author : Youhei Sato
														 Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/
#include <d3d11.h>
#include <DirectXMath.h>
#include "DirectXTex.h"
using namespace DirectX;
#include "direct3d.h"
#include "shader.h"
#include <iostream>
#include "texture.h"
#include "sprite.h"



static constexpr int NUM_VERTEX = 4; // 頂点数


static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ


// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;


// 頂点構造体
struct Vertex
{
	XMFLOAT3 position; // 頂点座標
	XMFLOAT4 color;    // カラー
	XMFLOAT2 texcoord;
};


bool Sprite_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// デバイスとデバイスコンテキストのチェック
	if (!pDevice || !pContext) {
		std::cout << "Sprite_Initialize() : 与えられたデバイスかコンテキストが不正です" << std::endl;
		return 0;
	}

	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * NUM_VERTEX;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT result = g_pDevice->CreateBuffer(&bd, NULL, &g_pVertexBuffer);

	if (result != S_OK) throw 0;

	return true;
}

void Sprite_Finalize(void)
{
	SAFE_RELEASE(g_pVertexBuffer);
}

void Sprite_Draw(int tex_id, float x, float y, float w, float h, DirectX::XMFLOAT4 color)
{
	// シェーダーを描画パイプラインに設定
	Shader_Begin();

	// 頂点バッファをロックする
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// 頂点バッファへの仮想ポインタを取得
	Vertex* v = (Vertex*)msr.pData;

	// 頂点情報を書き込み
	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();


	v[0].position = { x, y, 0.0f };
	v[1].position = { x + w, y, 0.0f };
	v[2].position = { x, y + h, 0.0f };
	v[3].position = { x + w, y + h, 0.0f };


	v[0].color = color;
	v[1].color = color;
	v[2].color = color;
	v[3].color = color;



	v[0].texcoord = { 0.0f, 0.0f };
	v[1].texcoord = { 1.0f, 0.0f };
	v[2].texcoord = { 0.0f, 1.0f };
	v[3].texcoord = { 1.0f, 1.0f };



	// 頂点バッファのロックを解除
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	
	//頂点シェーダーにWorld変換行列を設定
	Shader_SetWorldMatrix(XMMatrixIdentity());

	// 頂点シェーダーにProjection変換行列を設定
	Shader_SetProjectionMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));


	Shader_SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	// プリミティブトポロジ設定
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//テクスチャの設定
	Texture_SetTexture(tex_id);


	// ポリゴン描画命令発行
	g_pContext->Draw(NUM_VERTEX, 0);
}


void Sprite_Draw(int tex_id, float x, float y, float w, float h, int tx, int ty, int tw, int th, DirectX::XMFLOAT4 color)
{
	// シェーダーを描画パイプラインに設定
	Shader_Begin();

	// 頂点バッファをロックする
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// 頂点バッファへの仮想ポインタを取得
	Vertex* v = (Vertex*)msr.pData;

	// 頂点情報を書き込み
	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();


	v[0].position = { x, y, 0.0f };
	v[1].position = { x + w, y, 0.0f };
	v[2].position = { x, y + h, 0.0f };
	v[3].position = { x + w, y + h, 0.0f };


	v[0].color = color;
	v[1].color = color;
	v[2].color = color;
	v[3].color = color;


	float texw = static_cast<float>(Texture_GetWidth(tex_id));
	float texh = static_cast<float>(Texture_GetHeight(tex_id));

	float u0 = (float)(tx) / texw;
	float v0 = (float)(ty) / texh;
	float u1 = (float)(tx + tw) / texw;
	float v1 = (float)(ty + th) / texh;

	v[0].texcoord = { u0, v0 };
	v[1].texcoord = { u1, v0 };
	v[2].texcoord = { u0, v1 };
	v[3].texcoord = { u1, v1 };




	// 頂点バッファのロックを解除
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);



	Shader_SetWorldMatrix(XMMatrixIdentity());

	// 頂点シェーダーにProjection変換行列を設定
	Shader_SetProjectionMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));


	Shader_SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	// プリミティブトポロジ設定
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//テクスチャの設定
	Texture_SetTexture(tex_id);


	// ポリゴン描画命令発行
	g_pContext->Draw(NUM_VERTEX, 0);
}


void Sprite_Draw(int tex_id, float x, float y, float w, float h, float angle, int tx, int ty, int tw, int th, DirectX::XMFLOAT4 color)
{
	// シェーダーを描画パイプラインに設定
	Shader_Begin();

	// 頂点バッファをロックする
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// 頂点バッファへの仮想ポインタを取得
	Vertex* v = (Vertex*)msr.pData;

	// 頂点情報を書き込み
	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();


	v[0].position = { -0.5f, -0.5f, 0.0f };
	v[1].position = { 0.5f, -0.5f, 0.0f };
	v[2].position = { -0.5f,  0.5f, 0.0f };
	v[3].position = { 0.5f,  0.5f, 0.0f };


	v[0].color = color;
	v[1].color = color;
	v[2].color = color;
	v[3].color = color;


	float texw = static_cast<float>(Texture_GetWidth(tex_id));
	float texh = static_cast<float>(Texture_GetHeight(tex_id));

	float u0 = (float)(tx) / texw;
	float v0 = (float)(ty) / texh;
	float u1 = (float)(tx + tw) / texw;
	float v1 = (float)(ty + th) / texh;

	v[0].texcoord = { u0, v0 };
	v[1].texcoord = { u1, v0 };
	v[2].texcoord = { u0, v1 };
	v[3].texcoord = { u1, v1 };




	// 頂点バッファのロックを解除
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);


	//頂点シェーダーにWorld変換行列を設定

	XMMATRIX s = XMMatrixScaling(w, h, 1.0f);
	XMMATRIX r = XMMatrixRotationZ(angle);
	XMMATRIX t = XMMatrixTranslation(x, y, 0.0f);

	Shader_SetWorldMatrix(s * r * t);

	// 頂点シェーダーにProjection変換行列を設定
	Shader_SetProjectionMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));


	Shader_SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	// プリミティブトポロジ設定
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//テクスチャの設定
	Texture_SetTexture(tex_id);


	// ポリゴン描画命令発行
	g_pContext->Draw(NUM_VERTEX, 0);
}
