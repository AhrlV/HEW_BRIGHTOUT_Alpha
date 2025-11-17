/*=======================================================================================================


     Cubeの描画[cube.cpp]

                                                                              Author :  Ryosuke Kageyama
                                                                              Date   :  2025/10/16
========================================================================================================*/

#include <directXMath.h>
#include <iostream>
#include "direct3d.h"
#include "cube.h"
#include "shader3d.h"
#include "texture.h"


using namespace DirectX;


static constexpr int NUM_VERTEX = 4 * 6; // 頂点数
static constexpr int NUM_INDEX = 6 * 6;


static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ
static ID3D11Buffer* g_pIndexBuffer = nullptr; // インデックスバッファ


// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

typedef unsigned short INDEX_TYPE;


// 頂点構造体
struct Vertex3d
{
	XMFLOAT3 position; // 頂点座標
	XMFLOAT4 color;    // カラー
	XMFLOAT3 normal;
	XMFLOAT2 texcoord; // テクスチャ座標
	float texuse = 1.0f;
};


static Vertex3d g_CubeVertex[] 
{

	//前面
	{{-0.5, 0.5f,-0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,0.0f,-1.0f}, {0.25f, 1.0f / 3.0f}},
	{{ 0.5, 0.5f,-0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,0.0f,-1.0f}, { 0.5f, 1.0f / 3.0f}},
	{{-0.5,-0.5f,-0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,0.0f,-1.0f}, {0.25f, 2.0f / 3.0f}},
	{{ 0.5,-0.5f,-0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,0.0f,-1.0f}, { 0.5f, 2.0f / 3.0f}},

	//天井
	{{-0.5, 0.5f, 0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,1.0f,0.0f}, {0.25f,        0.0f}},
	{{ 0.5, 0.5f, 0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,1.0f,0.0f}, { 0.5f,        0.0f}},
	{{-0.5, 0.5f,-0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,1.0f,0.0f}, {0.25f, 1.0f / 3.0f}},
	{{ 0.5, 0.5f,-0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,1.0f,0.0f}, { 0.5f, 1.0f / 3.0f}},

	//右側面
	{{ 0.5, 0.5f,-0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {1.0f,0.0f,0.0f}, { 0.5f, 1.0f / 3.0f}},
	{{ 0.5, 0.5f, 0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {1.0f,0.0f,0.0f}, {0.75f, 1.0f / 3.0f}},
	{{ 0.5,-0.5f,-0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {1.0f,0.0f,0.0f}, { 0.5f, 2.0f / 3.0f}},
	{{ 0.5,-0.5f, 0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {1.0f,0.0f,0.0f}, {0.75f, 2.0f / 3.0f}},


	//背面
	{{ 0.5, 0.5f, 0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,0.0f,1.0f}, {0.75f, 1.0f / 3.0f}},
	{{-0.5, 0.5f, 0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,0.0f,1.0f}, { 1.0f, 1.0f / 3.0f}},
	{{ 0.5,-0.5f, 0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,0.0f,1.0f}, {0.75f, 2.0f / 3.0f}},
	{{-0.5,-0.5f, 0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,0.0f,1.0f}, { 1.0f, 2.0f / 3.0f}},

	//底面
	{{-0.5,-0.5f,-0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,-1.0f,0.0f}, {0.25f, 2.0f / 3.0f}},
	{{ 0.5,-0.5f,-0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,-1.0f,0.0f}, { 0.5f, 2.0f / 3.0f}},
	{{-0.5,-0.5f, 0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,-1.0f,0.0f}, {0.25f,        1.0f}},
	{{ 0.5,-0.5f, 0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {0.0f,-1.0f,0.0f}, { 0.5f,        1.0f}},

	//左側面
	{{-0.5, 0.5f, 0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {-1.0f,0.0f,0.0f}, { 0.0f, 1.0f / 3.0f}},
	{{-0.5, 0.5f,-0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {-1.0f,0.0f,0.0f}, {0.25f, 1.0f / 3.0f}},
	{{-0.5,-0.5f, 0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {-1.0f,0.0f,0.0f}, { 0.0f, 2.0f / 3.0f}},
	{{-0.5,-0.5f,-0.5f}, {1.0f,1.0f,1.0f,1.0f,}, {-1.0f,0.0f,0.0f}, {0.25f, 2.0f / 3.0f}},
};

static INDEX_TYPE g_CubeIndex[]
{
	 0,  1,  2,  2,  1,  3,
	 4,  5,  6,  6,  5,  7,
	 8,  9, 10, 10,  9, 11,
	12, 13, 14, 14, 13, 15,
	16, 17, 18, 18, 17, 19,
	20, 21, 22, 22, 21, 23
};

void Cube_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
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
	bd.ByteWidth = sizeof(Vertex3d) * NUM_VERTEX;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;


	// 頂点バッファへ流し込むデータの設定
	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = g_CubeVertex;

	HRESULT result_v = g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

	if (result_v != S_OK) throw 0;


	// インデックスバッファ生成
	bd.ByteWidth = sizeof(INDEX_TYPE) * NUM_INDEX;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;


	// インデックスバッファへ流し込むデータの設定
	sd.pSysMem = g_CubeIndex;


	HRESULT result_i = g_pDevice->CreateBuffer(&bd, &sd, &g_pIndexBuffer);

	if (result_i != S_OK) throw 0;

}

void Cube_Finalize()
{
	SAFE_RELEASE(g_pVertexBuffer);
	SAFE_RELEASE(g_pIndexBuffer);
}

void Cube_Draw(const DirectX::XMMATRIX& mtxWorld, int tex_id)
{
	// シェーダーを描画パイプラインに設定
	Shader3d_Begin();


	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3d);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);


	// 頂点インデックスを描画パイプラインに設定
	g_pContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);



	Shader3d_SetWorldMatrix(mtxWorld);

	Shader3d_SetMaterialDiffuse({ 1.0f, 1.0f, 1.0f, 1.0f });

	// プリミティブトポロジ設定
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//テクスチャの設定
	Texture_SetTexture(tex_id);

	// ポリゴン描画命令発行
	g_pContext->DrawIndexed(NUM_INDEX, 0, 0);

}
