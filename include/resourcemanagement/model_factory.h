/*============================================================================================================

    モデルファクトリー [model_factory.h]
    Model系リソースの生成・管理を行うファクトリークラス。
    ResourceManagerと連携して、リソースの一元管理とキャッシュを提供する。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#ifndef MODEL_FACTORY_H
#define MODEL_FACTORY_H

#include <memory>
#include <string>
#include <d3d11.h>

// 前方宣言
class Model;
class Prefab;

/*============================================================================================================
    ModelFactoryクラス
    Model系リソースの生成・管理を行うファクトリークラス。
    ResourceManagerに問い合わせ、キャッシュされていればそれを返し、
    なければファイルから読み込みResourceManagerに登録する。
    ファイルパスには自動的に"resources/model/"を先頭に付与される。
=============================================================================================================*/
class ModelFactory
{
public:

	/*========================================================================================================
		FBXファイルからPrefabを生成する
		
		AssimpでFBXファイルを読み込み、階層構造を持つPrefabを生成する。
		三角形化はせず、左手座標系に変換する。
		読み込んだMaterialとMeshの組み合わせごとにMeshRendererを持つGameObjectを生成し、
		階層構造を組み立てる。ルートGameObjectとして空のGameObjectを作成し、
		読み込んだModel名を付ける。
		PrefabはResourceManagerに".prefab"を付けて登録される。
		
		引数:
		  filename - FBXファイルのパス（"resources/model/"は自動付与）
		戻り値: 生成されたPrefabのshared_ptr
		例外: 読み込みに失敗した場合はruntime_errorをスロー
	========================================================================================================*/
	static std::shared_ptr<Prefab> LoadFromFBX(const std::wstring& filename, float scale = 1.0f);

private:
	// コンストラクタ削除（静的クラスとして使用）
	ModelFactory() = delete;
	
	/*========================================================================================================
		ファイルパスに"resources/model/"プレフィックスを付ける
		
		引数:
		  filename - 元のファイル名
		戻り値: プレフィックスが付与されたフルパス
	========================================================================================================*/
	static std::wstring BuildModelPath(const std::wstring& filename);
};

#endif // MODEL_FACTORY_H
