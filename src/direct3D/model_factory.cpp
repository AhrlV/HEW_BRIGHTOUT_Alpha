/*============================================================================================================

    モデルファクトリー実装 [model_factory.cpp]
    Modelリソースの生成を管理するファクトリークラスの実装。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#include "direct3D/model_factory.h"
#include "direct3D/model.h"
#include "direct3D/resource_manager.h"
#include "direct3D/direct3d_device.h"
#include <stdexcept>

/*========================================================================================================
	ModelFactory::BuildModelPath
	
	ファイルパスに"resources/model/"プレフィックスを付加する。
	
	引数:
	  filename - 元のファイル名
	戻り値: プレフィックスが付加されたフルパス
========================================================================================================*/
std::wstring ModelFactory::BuildModelPath(const std::wstring& filename)
{
	return L"resources/model/" + filename;
}

/*========================================================================================================
	ModelFactory::Create
	
	モデルの生成または取得を行う。
	ResourceManagerにキャッシュされていればそれを返し、
	なければファイルから読み込んでResourceManagerに登録する。
	キャッシュキーはファイル名のみで、フルパスは含まない。
	
	引数:
	  filename - モデルファイルのパス（"resources/model/"は自動付与）
	戻り値: モデルのshared_ptr
	例外: 読み込みに失敗した場合はruntime_errorをスロー
========================================================================================================*/
std::shared_ptr<Model> ModelFactory::Create(const std::wstring& filename)
{

	// ResourceManagerから取得を試みる（ファイル名をキーとして使用）
	auto& manager = ResourceManager::Instance();
	auto resource = manager.GetResource<Model>(filename);
	
	if (resource)
	{
		// キャッシュされていたらそれを返す
		return resource;
	}
	
	// キャッシュされていない場合は新規作成
	auto model = std::make_shared<Model>(filename);
	
	// フルパスを生成
	std::wstring fullPath = BuildModelPath(filename);
	
	// ファイルから読み込み（プレースホルダー）
	// TODO: 実際のモデル読み込み処理を実装する
	// model->Initialize(fullPath);
	
	// ResourceManagerに登録（ファイル名をキーとして登録）
	manager.RegisterResource(filename, model);
	
	throw std::runtime_error("ModelFactory::Create - モデルの読み込みは未実装です");
}
