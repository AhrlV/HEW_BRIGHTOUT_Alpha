/*============================================================================================================

    マテリアルファクトリー実装 [material_factory.cpp]
    Materialリソースの生成を管理するファクトリークラスの実装。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#include "resourcemanagement/material_factory.h"
#include "resourcemanagement/material.h"
#include "resourcemanagement/resource_manager.h"
#include <stdexcept>
#include "direct3D/direct3d_device.h"

/*========================================================================================================
	MaterialFactory::Create
	
	マテリアルの生成または取得を行う。
	ResourceManagerにキャッシュされていればそれを返し、
	なければ新規作成してResourceManagerに登録する。
	キャッシュキーはファイル名のみで、フルパスは含まない。
	
	引数:
	  filename - マテリアルファイルのパス
	戻り値: マテリアルのshared_ptr
	例外: 読み込みに失敗した場合はruntime_errorをスロー
========================================================================================================*/
std::shared_ptr<Material> MaterialFactory::Create(const std::wstring& filename)
{
	// GraphicsDevice取得
	auto& device = GraphicsDevice::Instance();
	auto d3dDevice = device.GetDevice();

	if (!d3dDevice)
	{
		throw std::runtime_error("MaterialFactory::Create - デバイスがnullです");
	}

	// ResourceManagerから取得を試みる（ファイル名をキーとして使用）
	auto& manager = ResourceManager::Instance();
	auto resource = manager.GetResource<Material>(filename);
	
	if (resource)
	{
		// キャッシュされていたらそれを返す
		return resource;
	}
	
	// キャッシュされていない場合は新規作成
	auto material = std::make_shared<Material>(filename);
	
	// デフォルトの初期化（コンストラクタで実行済み）
	if (!material->Initialize())
	{
		throw std::runtime_error("MaterialFactory::Create - マテリアルの初期化に失敗しました");
	}
	
	// ResourceManagerに登録（ファイル名をキーとして登録）
	manager.RegisterResource(filename, material);
	
	return material;
}
