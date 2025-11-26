/*============================================================================================================

    テクスチャファクトリー実装 [texture_factory.cpp]
    Textureリソースの生成と管理を行うファクトリークラスの実装。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#include "direct3D/texture_factory.h"
#include "direct3D/texture.h"
#include "direct3D/resource_manager.h"
#include "direct3D/direct3d_device.h"
#include "direct3D/DirectXTex.h"
#include <stdexcept>

#ifdef _DEBUG
#pragma comment(lib, "DirectXTex/DirectXTex_Debug.lib")
#else
#pragma comment(lib, "DirectXTex.lib")
#endif

/*========================================================================================================
	TextureFactory::BuildTexturePath
	
	ファイルパスに"resources/texture/"プレフィックスを付加する。
	
	引数:
	  filename - 元のファイル名
	戻り値: プレフィックスが付加されたフルパス
========================================================================================================*/
std::wstring TextureFactory::BuildTexturePath(const std::wstring& filename)
{
	return L"resources/texture/" + filename;
}

/*========================================================================================================
	TextureFactory::Create
	
	テクスチャの生成または取得を行う。
	ResourceManagerにキャッシュされていればそれを返し、
	なければファイルから読み込んでResourceManagerに登録する。
	キャッシュキーはファイル名のみ（ミップマップフラグを含む）で、フルパスは含まない。
	
	引数:
	  filename - テクスチャファイルのパス（"resources/texture/"は自動付与）
	  generateMipMaps - ミップマップを生成するかどうか
	戻り値: テクスチャのshared_ptr
	例外: 読み込みに失敗した場合はruntime_errorをスロー
========================================================================================================*/
std::shared_ptr<Texture> TextureFactory::Create(const std::wstring& filename, bool generateMipMaps)
{

	// キャッシュキーを生成（ミップマップフラグを含める）
	std::wstring cacheKey = filename;
	if (generateMipMaps)
	{
		cacheKey += L"/m";
	}

	// ResourceManagerから取得を試みる
	auto& manager = ResourceManager::Instance();
	auto resource = manager.GetResource<Texture>(cacheKey);
	
	if (resource)
	{
		// キャッシュされていたらそれを返す
		return resource;
	}
	
	// キャッシュされていない場合は新規作成
	auto texture = std::make_shared<Texture>(cacheKey);
	
	// フルパスを生成
	std::wstring fullPath = BuildTexturePath(filename);
	
	// ファイルから読み込み（Factoryで実行）
	// GraphicsDevice取得
	auto& device = GraphicsDevice::Instance();
	auto d3dDevice = device.GetDevice();
	
	if (!d3dDevice)
	{
		throw std::runtime_error("TextureFactory::Create - デバイスがnullです");
	}
	
	if (fullPath.empty())
	{
		throw std::runtime_error("TextureFactory::Create - ファイル名が空です");
	}

	// テクスチャメタデータとイメージデータ
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage image;

	// 画像ファイルの読み込み
	HRESULT hr = DirectX::LoadFromWICFile(fullPath.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, image);
	if (FAILED(hr))
	{
		throw std::runtime_error("TextureFactory::Create - 画像ファイルの読み込みに失敗しました");
	}

	// 画像サイズを保存
	unsigned int width = static_cast<unsigned int>(metadata.width);
	unsigned int height = static_cast<unsigned int>(metadata.height);

	// ミップマップの生成
	if (generateMipMaps)
	{
		DirectX::ScratchImage mipChain;
		hr = DirectX::GenerateMipMaps(
			image.GetImages(),
			image.GetImageCount(),
			image.GetMetadata(),
			DirectX::TEX_FILTER_DEFAULT,
			0,
			mipChain
		);

		if (SUCCEEDED(hr))
		{
			image = std::move(mipChain);
			metadata = image.GetMetadata();
		}
	}

	// シェーダーリソースビューの作成
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	hr = DirectX::CreateShaderResourceView(
		d3dDevice,
		image.GetImages(),
		image.GetImageCount(),
		metadata,
		srv.GetAddressOf()
	);

	if (FAILED(hr))
	{
		throw std::runtime_error("TextureFactory::Create - シェーダーリソースビューの作成に失敗しました");
	}

	// Textureオブジェクトにデータを設定
	texture->SetTextureData(srv, width, height);
	
	// ResourceManagerに登録（キャッシュキーで登録）
	manager.RegisterResource(cacheKey, texture);
	
	return texture;
}
