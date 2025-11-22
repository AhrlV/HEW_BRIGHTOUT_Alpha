/*=======================================================================================================


	 Gridの描画[grid.cpp]

																			  Author :  Ryosuke Kageyama
																			  Date   :  2025/10/21
========================================================================================================*/

#include <directXMath.h>
#include <iostream>
#include "direct3d.h"
#include "grid.h"
#include "shader3d.h"


using namespace DirectX;



static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ


// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

static int g_Num_vertex; //頂点数

// 頂点構造体
struct Vertex3d
{
	XMFLOAT3 position{}; // 頂点座標
	XMFLOAT4 color{};    // カラー
	XMFLOAT3 normal{0.0f,0.0f,0.0f};
	XMFLOAT2 texcoord = {0.0f, 0.0f};
	float texuse = 0.0f;
};

void Grid_Init(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, int x_count, int z_count, float grid_edge_x, float grid_edge_z)
{
	const int   NUM_GRID_X = x_count;
	const int   NUM_GRID_Z = z_count;
	const float GRID_EDGE_X = grid_edge_x;
	const float GRID_EDGE_Z = grid_edge_z;
	g_Num_vertex = (NUM_GRID_X + 1 + NUM_GRID_Z + 1) * 2; // 頂点数


	auto* pV = new Vertex3d[g_Num_vertex];

	for (int i = 0; i < (NUM_GRID_X + 1) * 2; i += 2)
	{
		float x = NUM_GRID_X * GRID_EDGE_X / 2 - GRID_EDGE_X * i / 2;
		float z = NUM_GRID_Z * GRID_EDGE_Z / 2;

		XMFLOAT4 color;
		color = { 1.0f,1.0f,1.0f,1.0f };
		if (x == 0.0f) color = { 0.0f, 0.0f,1.0f,1.0f };

		pV[i]     = { {x, 0.0f, z},color};
		pV[i + 1] = { {x, 0.0f,-z},color};
	}

	for (int i = (NUM_GRID_X + 1) * 2; i < g_Num_vertex; i += 2)
	{
		float x = NUM_GRID_Z * GRID_EDGE_Z / 2;
		float z = NUM_GRID_Z * GRID_EDGE_Z / 2 - GRID_EDGE_Z * (i - (NUM_GRID_X + 1.0f) * 2) / 2;

		XMFLOAT4 color;
		color = { 1.0f,1.0f,1.0f,1.0f };
		if (z == 0.0f) color = { 1.0f, 0.0f,0.0f,1.0f };

		pV[i]     = { { x, 0.0f, z},color};
		pV[i + 1] = { {-x, 0.0f, z},color};
	}



	// デバイスとデバイスコンテキストのチェック
	if (!pDevice || !pContext) {
		std::cout << "Sprite_Initialize() : 与えられたデバイスかコンテキストが不正です" << std::endl;
		return;
	}

	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex3d) * g_Num_vertex;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;





	// 頂点バッファへ流し込むデータの設定
	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = pV;



	HRESULT result = g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

	if (FAILED(result)) throw 0;

	delete[] pV;
}

void Grid_UnInit()
{
	SAFE_RELEASE(g_pVertexBuffer);
}

void Grid_Draw()
{
	// シェーダーを描画パイプラインに設定
	Shader3d_Begin();


	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3d);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);


	//頂点シェーダーにWorld変換行列を設定
	Shader3d_SetWorldMatrix(XMMatrixIdentity());


	// プリミティブトポロジ設定
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);


	// ポリゴン描画命令発行
	g_pContext->Draw(g_Num_vertex, 0);
}
