/*============================================================================================================

    モデルファクトリー実装 [model_factory.cpp]
    Model系リソースの生成・管理を行うファクトリークラスの実装。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#include "resourcemanagement/model_factory.h"
#include "resourcemanagement/prefab.h"
#include "resourcemanagement/resource_manager.h"
#include "resourcemanagement/mesh.h"
#include "resourcemanagement/material.h"
#include "resourcemanagement/texture_factory.h"
#include "resourcemanagement/texture.h"
#include "lifecycle/gameobject.h"
#include "rendering/mesh_renderer.h"
#include "direct3D/direct3d_device.h"
#include "math/vector3.h"
#include "math/quaternion.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <DirectXTex.h>
#include <Windows.h>
#include <stdexcept>
#include <cmath>
#include <unordered_map>
#include <unordered_set>

/*========================================================================================================
	文字列変換ヘルパー関数
	std::wstringをstd::stringに変換する（Windows API使用）
========================================================================================================*/
static std::string WStringToString(const std::wstring& wstr)
{
	if (wstr.empty())
	{
		return std::string();
	}

	int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	std::string result(sizeNeeded - 1, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], sizeNeeded, nullptr, nullptr);
	return result;
}

/*========================================================================================================
	文字列変換ヘルパー関数
	std::stringをstd::wstringに変換する（Windows API使用）
========================================================================================================*/
static std::wstring StringToWString(const std::string& str)
{
	if (str.empty())
	{
		return std::wstring();
	}

	int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	std::wstring result(sizeNeeded - 1, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], sizeNeeded);
	return result;
}

/*========================================================================================================
	Assimp行列から位置・回転・スケールを抽出する
	
	Assimpの4x4変換行列（列優先）から、位置・回転・スケールを分解する。
	左手座標系への変換も同時に行う（Z軸を反転）。
	
	引数:
	  transform - Assimpの変換行列
	  outPosition - 抽出された位置（出力）
	  outRotation - 抽出された回転（出力）
	  outScale - 抽出されたスケール（出力）
	  scale - モデル全体のスケール係数
========================================================================================================*/
static void DecomposeTransform(
	const aiMatrix4x4& transform,
	Vector3& outPosition,
	Quaternion& outRotation,
	Vector3& outScale,
	float scale
)
{
	// 位置を抽出（4列目）
	// スケールを適用
	outPosition.x = transform.a4 * scale;
	outPosition.y = transform.b4 * scale;
	outPosition.z = transform.c4 * scale;

	// 各軸ベクトルを抽出
	Vector3 axisX(transform.a1, transform.b1, transform.c1);
	Vector3 axisY(transform.a2, transform.b2, transform.c2);
	Vector3 axisZ(transform.a3, transform.b3, transform.c3);

	// スケールを抽出（各軸ベクトルの長さ）
	outScale.x = axisX.Length();
	outScale.y = axisY.Length();
	outScale.z = axisZ.Length();

	// スケールが0の場合は1に設定（ゼロ除算防止）
	if (outScale.x < 0.0001f)
	{
		outScale.x = 1.0f;
	}
	if (outScale.y < 0.0001f)
	{
		outScale.y = 1.0f;
	}
	if (outScale.z < 0.0001f)
	{
		outScale.z = 1.0f;
	}

	// 回転行列を抽出（スケールを除去）
	axisX = axisX / outScale.x;
	axisY = axisY / outScale.y;
	axisZ = axisZ / outScale.z;

	// 回転行列からクォータニオンを計算
	DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixIdentity();
	rotMatrix.r[0] = DirectX::XMVectorSet(axisX.x, axisX.y, axisX.z, 0.0f);
	rotMatrix.r[1] = DirectX::XMVectorSet(axisY.x, axisY.y, axisY.z, 0.0f);
	rotMatrix.r[2] = DirectX::XMVectorSet(axisZ.x, axisZ.y, axisZ.z, 0.0f);
	rotMatrix.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	// 行列からクォータニオンに変換
	DirectX::XMVECTOR quatVec = DirectX::XMQuaternionRotationMatrix(rotMatrix);
	DirectX::XMFLOAT4 quatFloat;
	DirectX::XMStoreFloat4(&quatFloat, quatVec);

	// Quaternionオブジェクトを構築
	outRotation = Quaternion(quatFloat.x, quatFloat.y, quatFloat.z, quatFloat.w);
}

/*========================================================================================================
	aiMeshからMeshオブジェクトを作成する
	Assimpのメッシュデータから、エンジン内部のMeshオブジェクトを生成する。
	
	引数:
	  aiMeshPtr - Assimpのメッシュデータ
	  meshName - 作成するメッシュの名前
	  scale - モデル全体のスケール係数
	戻り値: 作成されたMeshのshared_ptr
	例外: メッシュ作成に失敗した場合はruntime_errorをスロー
========================================================================================================*/
static std::shared_ptr<Mesh> CreateMeshFromAssimp(const aiMesh* aiMeshPtr, const std::wstring& meshName, float scale)
{
	if (!aiMeshPtr)
	{
		throw std::runtime_error("CreateMeshFromAssimp: aiMesh is null");
	}

	// 頂点データを変換
	std::vector<MeshVertex> vertices;
	vertices.reserve(aiMeshPtr->mNumVertices);

	for (unsigned int i = 0; i < aiMeshPtr->mNumVertices; i++)
	{
		MeshVertex vertex = {};
		
		// 位置（スケールを適用）
		vertex.position = DirectX::XMFLOAT4(
			aiMeshPtr->mVertices[i].x * scale,
			aiMeshPtr->mVertices[i].y * scale,
			aiMeshPtr->mVertices[i].z * scale,
			1.0f
		);
		
		// 法線
		if (aiMeshPtr->HasNormals())
		{
			vertex.normal = DirectX::XMFLOAT3(
				aiMeshPtr->mNormals[i].x,
				aiMeshPtr->mNormals[i].y,
				aiMeshPtr->mNormals[i].z
			);
		}
		else
		{
			vertex.normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		}
		
		// UV座標（最初のセットのみ使用）
		if (aiMeshPtr->HasTextureCoords(0))
		{
			vertex.texcoord = DirectX::XMFLOAT2(
				aiMeshPtr->mTextureCoords[0][i].x,
				aiMeshPtr->mTextureCoords[0][i].y
			);
		}
		else
		{
			vertex.texcoord = DirectX::XMFLOAT2(0.0f, 0.0f);
		}
		
		// 頂点カラー（最初のセットのみ使用）
		if (aiMeshPtr->HasVertexColors(0))
		{
			vertex.color = DirectX::XMFLOAT4(
				aiMeshPtr->mColors[0][i].r,
				aiMeshPtr->mColors[0][i].g,
				aiMeshPtr->mColors[0][i].b,
				aiMeshPtr->mColors[0][i].a
			);
		}
		else
		{
			vertex.color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		
		vertices.push_back(vertex);
	}

	// インデックスデータを変換
	std::vector<uint32_t> indices;
	for (unsigned int i = 0; i < aiMeshPtr->mNumFaces; i++)
	{
		const aiFace& face = aiMeshPtr->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// Meshオブジェクトを作成
	auto mesh = std::make_shared<Mesh>(meshName);
	if (!mesh->Initialize(vertices, indices))
	{
		throw std::runtime_error("CreateMeshFromAssimp: Mesh initialization failed");
	}

	return mesh;
}

/*========================================================================================================
	aiMaterialからMaterialオブジェクトを作成する
	Assimpのマテリアルデータから、エンジン内部のMaterialオブジェクトを生成する。
	埋め込みテクスチャがある場合はDirectXTexのLoadFromWICMemoryを使用してメモリから直接ロードし、
	外部ファイルの場合はファイルパスから読み込む。
	
	引数:
	  aiMatPtr - Assimpのマテリアルデータ
	  scene - Assimpのシーン（テクスチャ情報取得用）
	  materialName - 作成するマテリアルの名前
	  textureMap - 読み込んだテクスチャのマップ（キャッシュ用）
	戻り値: 作成されたMaterialのshared_ptr
	例外: マテリアル作成に失敗した場合はruntime_errorをスロー
========================================================================================================*/
static std::shared_ptr<Material> CreateMaterialFromAssimp(
	const aiMaterial* aiMatPtr,
	[[maybe_unused]] const aiScene* scene,
	const std::wstring& materialName,
	std::unordered_map<std::string, std::shared_ptr<Texture>>& textureMap
)
{
	if (!aiMatPtr)
	{
		throw std::runtime_error("CreateMaterialFromAssimp: aiMaterial is null");
	}

	// Materialオブジェクトを作成
	auto material = std::make_shared<Material>(materialName);
	if (!material->Initialize())
	{
		throw std::runtime_error("CreateMaterialFromAssimp: Material initialization failed");
	}

	// ディフューズカラーを取得
	aiColor4D diffuse(1.0f, 1.0f, 1.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(aiMatPtr, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
	{
		material->DiffuseColor = DirectX::XMFLOAT4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
	}

	// アンビエントカラーを取得
	aiColor4D ambient(0.2f, 0.2f, 0.2f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(aiMatPtr, AI_MATKEY_COLOR_AMBIENT, &ambient))
	{
		material->AmbientColor = DirectX::XMFLOAT4(ambient.r, ambient.g, ambient.b, ambient.a);
	}

	// スペキュラーカラーを取得
	aiColor4D specular(1.0f, 1.0f, 1.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(aiMatPtr, AI_MATKEY_COLOR_SPECULAR, &specular))
	{
		material->SpecularColor = DirectX::XMFLOAT4(specular.r, specular.g, specular.b, specular.a);
	}

	// 光沢度を取得
	float shininess = 32.0f;
	if (AI_SUCCESS == aiGetMaterialFloat(aiMatPtr, AI_MATKEY_SHININESS, &shininess))
	{
		material->Shininess = shininess;
	}

	// 不透明度を取得
	float opacity = 1.0f;
	if (AI_SUCCESS == aiGetMaterialFloat(aiMatPtr, AI_MATKEY_OPACITY, &opacity))
	{
		material->Opacity = opacity;
	}

	// ディフューズテクスチャを取得
	aiString texturePath;
	if (AI_SUCCESS == aiGetMaterialTexture(aiMatPtr, aiTextureType_DIFFUSE, 0, &texturePath, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr))
	{
		std::string texturePathStr = texturePath.C_Str();
		
		// テクスチャがすでに読み込まれているかチェック
		if (texturePathStr.length() > 0)
		{
			auto it = textureMap.find(texturePathStr);
			if (it != textureMap.end())
			{
				// キャッシュされているテクスチャを使用
				material->Texture = it->second;
			}
			else
			{
				// 外部ファイルのテクスチャパス
				std::wstring texturePathW = StringToWString(texturePathStr);
				auto texture = TextureFactory::Create(texturePathW);
				material->Texture = texture;
				textureMap[texturePathStr] = texture;
			}
		}
	}

	return material;
}

/*========================================================================================================
	ModelFactory::BuildModelPath
	
	ファイルパスに"resources/model/"プレフィックスを付加する。
	
	引数:
	  filename - 元のファイル名
	戻り値: プレフィックスが付与されたフルパス
========================================================================================================*/
std::wstring ModelFactory::BuildModelPath(const std::wstring& filename)
{
	return L"resources/model/" + filename;
}

/*========================================================================================================
	aiNode階層をGameObjectの階層に変換する再帰関数
	
	Prefab用のGameObjectを作成し、TransFormによる階層関係を構築する。
	Scene登録はスキップするが、親子関係は正しく設定される。
	位置・回転・スケールを完全に抽出して適用する。
	
	引数:
	  node - Assimpのノード
	  scene - Assimpのシーン
	  parentGameObject - 親GameObject（階層構造設定用）
	  textureMap - 読み込んだテクスチャのマップ（キャッシュ用）
	  outAllGameObjects - 作成したすべてのGameObjectを格納するベクター
	  processedMeshes - 処理済みメッシュインデックスのセット（重複防止用）
	  scale - モデル全体のスケール係数
	戻り値: 生成されたGameObject（メッシュがない場合は空のGameObject）
========================================================================================================*/
static GameObject* ProcessNode(
	const aiNode* node,
	const aiScene* scene,
	GameObject* parentGameObject,
	std::unordered_map<std::string, std::shared_ptr<Texture>>& textureMap,
	std::vector<std::shared_ptr<GameObject>>& outAllGameObjects,
	std::unordered_set<unsigned int>& processedMeshes,
	float scale
)
{
	if (!node)
	{
		return nullptr;
	}

	// ノード名を取得（aiStringからstd::wstringに変換）
	std::string nodeName = std::string(node->mName.C_Str());
	std::wstring nodeNameW = StringToWString(nodeName);

	// このノード用のGameObjectを作成（shared_ptrで管理）
	// 重要: Prefab用なのでSceneには登録しない！
	auto gameObject = std::make_shared<GameObject>(true);
	
	// ノード名を設定
	if (!nodeNameW.empty())
	{
		gameObject->SetName(nodeNameW);
	}

	// TransFormを取得
	auto tf = gameObject->GetComponent<TransForm>();
	if (!tf)
	{
		throw std::runtime_error("ProcessNode: TransForm component is missing");
	}

	// 親子関係を設定
	if (parentGameObject)
	{
		auto parentTf = parentGameObject->GetComponent<TransForm>();
		if (parentTf)
		{
			tf->SetParent(parentTf);
		}
	}

	// TransForm行列を分解して位置・回転・スケールを抽出
	Vector3 position;
	Quaternion rotation;
	Vector3 nodeScale;
	DecomposeTransform(node->mTransformation, position, rotation, nodeScale, scale);

	// TransFormに設定
	tf->Position() = position;
	tf->Rotation() = rotation;
	tf->Scale() = nodeScale;

	// このGameObjectをリストに追加
	outAllGameObjects.push_back(gameObject);

	// このノードに関連付けられたメッシュを処理
	// 各メッシュは独立したGameObjectとして作成する
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		unsigned int meshIndex = node->mMeshes[i];
		aiMesh* aiMeshPtr = scene->mMeshes[meshIndex];
		
		// 処理済みメッシュセットに追加
		processedMeshes.insert(meshIndex);
		
		// メッシュ用の子GameObjectを作成
		auto meshGameObject = std::make_shared<GameObject>(true);
		
		// メッシュ名を生成
		std::wstring meshName = nodeNameW + L"_Mesh_" + std::to_wstring(i);
		meshGameObject->SetName(meshName);
		
		// メッシュGameObjectのTransFormを取得し、親子関係を設定
		auto meshTf = meshGameObject->GetComponent<TransForm>();
		if (meshTf)
		{
			// このノードのGameObjectを親として設定
			meshTf->SetParent(tf);
		}
		
		// Meshを作成（スケールを適用）
		std::wstring fullMeshName = meshName + L"_MeshData";
		auto mesh = CreateMeshFromAssimp(aiMeshPtr, fullMeshName, scale);
		
		// Materialを作成
		std::wstring materialName = meshName + L"_Material";
		auto material = CreateMaterialFromAssimp(
			scene->mMaterials[aiMeshPtr->mMaterialIndex],
			scene,
			materialName,
			textureMap
		);
		
		// MeshRendererを追加
		auto meshRenderer = meshGameObject->AddComponent<MeshRenderer>();
		meshRenderer->SetMesh(mesh);
		meshRenderer->SetMaterial(material);
		
		// メッシュGameObjectをリストに追加
		outAllGameObjects.push_back(meshGameObject);
	}

	// 子ノードを再帰的に処理
	// 親として現在のGameObjectを渡す
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(
			node->mChildren[i],
			scene,
			gameObject.get(),
			textureMap,
			outAllGameObjects,
			processedMeshes,
			scale
		);
	}
	
	return gameObject.get();
}

/*========================================================================================================
	ModelFactory::LoadFromFBX
	
	FBXファイルからPrefabを生成する。
	Assimpを使用してFBXファイルを読み込み、階層構造を持つPrefabを作成する。
	Prefab用のGameObjectはScene登録をスキップする。
	model.cppの成功している手順を参考に、埋め込みテクスチャを読み込む。
	階層化されていないメッシュ（scene->mMeshesから直接取得）も処理する。
	
	引数:
	  filename - FBXファイルのパス（"resources/model/"は自動付与）
	  scale - モデル全体のスケール係数（デフォルト: 1.0f）
	戻り値: 生成されたPrefabのshared_ptr
	例外: 読み込みに失敗した場合はruntime_errorをスロー
========================================================================================================*/
std::shared_ptr<Prefab> ModelFactory::LoadFromFBX(const std::wstring& filename, float scale)
{
	// Prefabのキャッシュキーを生成（.prefab拡張子を付加)
	std::wstring prefabKey = filename + L".prefab";
	
	// ResourceManagerからPrefabを取得を試みる
	auto& manager = ResourceManager::Instance();
	auto cachedPrefab = manager.GetResource<Prefab>(prefabKey);
	
	if (cachedPrefab)
	{
		// キャッシュされていたらそれを返す
		return cachedPrefab;
	}

	// フルパスを生成
	std::wstring fullPath = BuildModelPath(filename);
	std::string fullPathStr = WStringToString(fullPath);

	// Assimpインポーターを作成
	Assimp::Importer importer;
	
	// FBXファイルを読み込む
	// aiProcess_ConvertToLeftHanded: 左手座標系に変換
	const aiScene* scene = importer.ReadFile(
		fullPathStr,
		aiProcessPreset_TargetRealtime_MaxQuality | 
		aiProcess_ConvertToLeftHanded 
	);

	// 読み込みエラーチェック
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::string errorMsg = "ModelFactory::LoadFromFBX - Failed to load FBX file: ";
		errorMsg += fullPathStr;
		errorMsg += " Error: ";
		errorMsg += importer.GetErrorString();
		throw std::runtime_error(errorMsg);
	}

	// テクスチャマップを作成（model.cppと同じ手順で埋め込みテクスチャを読み込む）
	std::unordered_map<std::string, std::shared_ptr<Texture>> textureMap;
	
	// GraphicsDeviceを取得
	auto& device = GraphicsDevice::Instance();
	auto d3dDevice = device.GetDevice();
	
	if (!d3dDevice)
	{
		throw std::runtime_error("ModelFactory::LoadFromFBX - Device is null");
	}
	
	// 埋め込みテクスチャを読み込む（model.cppの手順を参考）
	for (unsigned int i = 0; i < scene->mNumTextures; i++)
	{
		aiTexture* aiTexturePtr = scene->mTextures[i];
		
		// テクスチャ名を生成
		std::string textureKey = aiTexturePtr->mFilename.data;
		std::wstring textureName = L"EmbeddedTexture_" + std::to_wstring(i);
		
		// Textureオブジェクトを作成
		auto texture = std::make_shared<Texture>(textureName);
		
		// DirectXTexを使用してメモリから読み込む（model.cppと同じ手順）
		DirectX::TexMetadata metadata;
		DirectX::ScratchImage image;
		
		HRESULT hr = DirectX::LoadFromWICMemory(
			reinterpret_cast<const uint8_t*>(aiTexturePtr->pcData),
			static_cast<size_t>(aiTexturePtr->mWidth),
			DirectX::WIC_FLAGS_NONE,
			&metadata,
			image
		);
		
		if (FAILED(hr))
		{
			throw std::runtime_error("ModelFactory::LoadFromFBX - Failed to load embedded texture from memory");
		}
		
		// ShaderResourceViewを作成（model.cppと同じ手順）
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
			throw std::runtime_error("ModelFactory::LoadFromFBX - Failed to create shader resource view for embedded texture");
		}
		
		// TextureオブジェクトにSRVを設定
		texture->SetTextureData(
			srv,
			static_cast<unsigned int>(metadata.width),
			static_cast<unsigned int>(metadata.height)
		);
		
		// テクスチャマップに追加
		textureMap[textureKey] = texture;
	}

	// ルートGameObjectを作成（Prefab用なのでSceneに登録しない）
	auto rootGameObject = std::make_shared<GameObject>(true);
	
	// モデル名を設定（ファイル名から拡張子を除いた名前）
	size_t dotPos = filename.find_last_of(L'.');
	std::wstring modelName = (dotPos != std::wstring::npos) ? filename.substr(0, dotPos) : filename;
	rootGameObject->SetName(modelName);

	// すべてのGameObjectを格納するベクター（ルートも含む）
	std::vector<std::shared_ptr<GameObject>> allGameObjects;
	allGameObjects.push_back(rootGameObject);

	// 処理済みメッシュインデックスを記録するセット
	std::unordered_set<unsigned int> processedMeshes;

	// ノード階層を再帰的に処理してGameObject階層を構築
	if (scene->mRootNode)
	{
		// ルートノードの子ノードを処理（ルートノード自体はスキップ）
		for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; i++)
		{
			ProcessNode(
				scene->mRootNode->mChildren[i],
				scene,
				rootGameObject.get(),
				textureMap,
				allGameObjects,
				processedMeshes,
				scale
			);
		}
	}

	// 階層化されていないメッシュを処理
	// scene->mMeshesから直接取得し、未処理のメッシュをルートGameObjectに追加
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		// すでに処理済みのメッシュはスキップ
		if (processedMeshes.find(i) != processedMeshes.end())
		{
			continue;
		}

		aiMesh* aiMeshPtr = scene->mMeshes[i];
		
		// 階層化されていないメッシュ用のGameObjectを作成
		auto meshGameObject = std::make_shared<GameObject>(true);
		
		// メッシュ名を設定
		std::string meshNameStr = aiMeshPtr->mName.C_Str();
		std::wstring meshNameW = StringToWString(meshNameStr);
		if (meshNameW.empty())
		{
			meshNameW = L"UnhierarchicalMesh_" + std::to_wstring(i);
		}
		meshGameObject->SetName(meshNameW);
		
		// TransFormを取得し、ルートGameObjectの子として設定
		auto meshTf = meshGameObject->GetComponent<TransForm>();
		if (meshTf)
		{
			auto rootTf = rootGameObject->GetComponent<TransForm>();
			if (rootTf)
			{
				meshTf->SetParent(rootTf);
			}
		}
		
		// Meshを作成（スケールを適用）
		std::wstring fullMeshName = meshNameW + L"_Mesh";
		auto mesh = CreateMeshFromAssimp(aiMeshPtr, fullMeshName, scale);
		
		// Materialを作成
		std::wstring materialName = meshNameW + L"_Material";
		auto material = CreateMaterialFromAssimp(
			scene->mMaterials[aiMeshPtr->mMaterialIndex],
			scene,
			materialName,
			textureMap
		);
		
		// MeshRendererを追加
		auto meshRenderer = meshGameObject->AddComponent<MeshRenderer>();
		meshRenderer->SetMesh(mesh);
		meshRenderer->SetMaterial(material);
		
		// GameObjectをリストに追加
		allGameObjects.push_back(meshGameObject);
	}

	// Prefabを作成
	auto prefab = std::make_shared<Prefab>();
	prefab->Initialize(rootGameObject, allGameObjects, modelName);

	// ResourceManagerに登録（.prefab拡張子付きで登録）
	manager.RegisterResource(prefabKey, prefab);

	return prefab;
}
