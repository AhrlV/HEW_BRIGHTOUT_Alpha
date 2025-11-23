
#include "direct3D/texture.h"
#include "direct3D/direct3d.h"
#include <string>
#include "direct3D/DirectXTex.h"
#include <unordered_map>
using namespace DirectX;

static constexpr int TEXTURE_MAX = 1024;

struct Texture
{
	std::wstring filename;
	unsigned int width = 0;
	unsigned int height = 0;
	ID3D11ShaderResourceView* pTexture = nullptr;
};

static Texture g_Textures[TEXTURE_MAX];

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

void Texture_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;
}

void Texture_Finalize()
{
	Texture_AllRelease();
}

int Texture_Load(const wchar_t* pFileName, bool isMipMap)
{
	static std::unordered_map<const wchar_t*, unsigned int> tex_umap;

	if (tex_umap.count(pFileName)) return tex_umap.at(pFileName);

	//すでに読み込んでいたファイル対応
	for (int i = 0; i < TEXTURE_MAX; i++) {
		if (!g_Textures[i].pTexture) continue;

		if (g_Textures[i].filename == pFileName) {
			return i;
		}
	}

	//管理処理
	for (int i = 0; i < TEXTURE_MAX; i++) {

		//空いている場所を探す
		if (g_Textures[i].pTexture) continue;

		//テクスチャからの読み込み
		TexMetadata metadata;
		ScratchImage image;

		//画像ファイルの読み込み
		LoadFromWICFile(pFileName, WIC_FLAGS_NONE, &metadata, image);

		//画像ファイルのサイズを取得
		g_Textures[i].width = (unsigned int)metadata.width;
		g_Textures[i].height = (unsigned int)metadata.height;


		if (isMipMap) {
			//ミップマップを作成する
			ScratchImage mipChain;
			GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(),
				TEX_FILTER_DEFAULT, 0, mipChain);
			//TEX_FILTER_BOX | TEX_FILTER_FORCE_NON_WIC
			image = std::move(mipChain);
			metadata = image.GetMetadata();
		}

		//シェーダーリソースビューの生成
		HRESULT hr = CreateShaderResourceView(g_pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Textures[i].pTexture);

		if (FAILED(hr)) {
			MessageBox(nullptr, "テクスチャの読み込みに失敗しました", "エラー", MB_OK);
			throw 0;
			break;
		}

		g_Textures[i].filename = pFileName;

		tex_umap[pFileName] = i;

		return i;
	}

	return -1;
}

void Texture_AllRelease()
{
	for (Texture& t : g_Textures) {
		SAFE_RELEASE(t.pTexture);
	}
}

void Texture_SetTexture(int texture_id)
{
	if (texture_id < 0) {
		return;
	}

	//テクスチャ設定
	g_pContext->PSSetShaderResources(0, 1, &g_Textures[texture_id].pTexture);

}

DirectX::XMUINT2 Texture_GetSize(int texture_id)
{
	return { g_Textures[texture_id].width, g_Textures[texture_id].height };
}

const unsigned int& Texture_GetWidth(int texture_id)
{
	return g_Textures[texture_id].width;
}

const unsigned int& Texture_GetHeight(int texture_id)
{
	return g_Textures[texture_id].height;
}
