/*============================================================================================================

    シェーダーファクトリー実装 [shader_factory.cpp]
    Shaderリソースの生成を管理するファクトリークラスの実装。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#include "direct3D/shader_factory.h"
#include "direct3D/shader.h"
#include "direct3D/resource_manager.h"
#include "direct3D/direct3d_device.h"
#include <stdexcept>
#include <fstream>

/*========================================================================================================
	ShaderFactory::BuildShaderPath
	
	ファイルパスに"shaders/cso/"プレフィックスを付加する。
	
	引数:
	  filename - 元のファイル名
	戻り値: プレフィックスが付加されたフルパス
========================================================================================================*/
std::wstring ShaderFactory::BuildShaderPath(const std::wstring& filename)
{
	return L"shaders/cso/" + filename;
}

/*========================================================================================================
	ShaderFactory::LoadShaderFile
	
	シェーダーファイル(.cso)を読み込む。
	コンパイル済みシェーダーファイルをバイナリとして読み込む。
	
	引数:
	  filename - シェーダーファイルのパス
	  bytecode - 読み込んだバイトコードを格納するベクター
	例外: 読み込みに失敗した場合はruntime_errorをスロー
========================================================================================================*/
void ShaderFactory::LoadShaderFile(const std::wstring& filename, std::vector<uint8_t>& bytecode)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		throw std::runtime_error("ShaderFactory::LoadShaderFile - ファイルが開けませんでした");
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	if (size <= 0)
	{
		throw std::runtime_error("ShaderFactory::LoadShaderFile - ファイルが空です");
	}

	bytecode.resize(static_cast<size_t>(size));
	if (!file.read(reinterpret_cast<char*>(bytecode.data()), size))
	{
		throw std::runtime_error("ShaderFactory::LoadShaderFile - ファイルの読み込みに失敗しました");
	}

	file.close();
}

/*========================================================================================================
	ShaderFactory::CreateVertexShader
	
	頂点シェーダーの生成または取得を行う。
	ResourceManagerにキャッシュされていればそれを返し、
	なければファイルから読み込んでResourceManagerに登録する。
	ファイルパスには自動的に"shaders/cso/"が先頭に付加される。
	
	引数:
	  filename - シェーダーファイルのパス（"shaders/cso/"は自動付加）
	  inputLayout - 入力レイアウト記述の配列
	戻り値: 頂点シェーダーのshared_ptr
	例外: 読み込みに失敗した場合はruntime_errorをスロー
========================================================================================================*/
std::shared_ptr<VertexShader> ShaderFactory::CreateVertexShader(
	const std::wstring& filename,
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayout)
{
	// GraphicsDeviceからDeviceを取得
	auto& device = GraphicsDevice::Instance();
	auto d3dDevice = device.GetDevice();
	
	if (!d3dDevice)
	{
		throw std::runtime_error("デバイスがnullです");
	}

	if (filename.empty())
	{
		throw std::runtime_error("ファイル名が空です");
	}

	if (inputLayout.empty())
	{
		throw std::runtime_error("入力レイアウトが空です");
	}

	// フルパス生成
	std::wstring fullPath = BuildShaderPath(filename);

	// ResourceManagerから取得を試みる
	auto& manager = ResourceManager::Instance();
	auto resource = manager.GetResource<VertexShader>(filename);
	
	if (resource)
	{
		// キャッシュされていたらそれを返す
		return resource;
	}
	
	// キャッシュされていない場合は新規作成
	auto vertexShader = std::make_shared<VertexShader>();
	
	// シェーダーバイトコードを読み込み
	std::vector<uint8_t> bytecode;
	LoadShaderFile(fullPath, bytecode);
	
	// 頂点シェーダーを作成
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
	HRESULT hr = d3dDevice->CreateVertexShader(
		bytecode.data(),
		bytecode.size(),
		nullptr,
		vs.GetAddressOf()
	);
	
	if (FAILED(hr))
	{
		throw std::runtime_error("頂点シェーダーの作成に失敗しました");
	}
	
	// 入力レイアウトを作成
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayoutObj;
	hr = d3dDevice->CreateInputLayout(
		inputLayout.data(),
		static_cast<UINT>(inputLayout.size()),
		bytecode.data(),
		bytecode.size(),
		inputLayoutObj.GetAddressOf()
	);
	
	if (FAILED(hr))
	{
		throw std::runtime_error("入力レイアウトの作成に失敗しました");
	}
	
	// VertexShaderオブジェクトの内部状態を設定
	// 注: 保護されたメンバーにアクセスする必要があるため、
	// Initialize関数を使用します
	if (!vertexShader->Initialize(fullPath, inputLayout))
	{
		throw std::runtime_error("頂点シェーダーの初期化に失敗しました");
	}
	
	// ResourceManagerに登録
	manager.RegisterResource(filename, vertexShader);
	
	return vertexShader;
}

/*========================================================================================================
	ShaderFactory::CreatePixelShader
	
	ピクセルシェーダーの生成または取得を行う。
	ResourceManagerにキャッシュされていればそれを返し、
	なければファイルから読み込んでResourceManagerに登録する。
	ファイルパスには自動的に"shaders/cso/"が先頭に付加される。
	
	引数:
	  filename - シェーダーファイルのパス（"shaders/cso/"は自動付加）
	戻り値: ピクセルシェーダーのshared_ptr
	例外: 読み込みに失敗した場合はruntime_errorをスロー
========================================================================================================*/
std::shared_ptr<PixelShader> ShaderFactory::CreatePixelShader(
	const std::wstring& filename)
{
	if (filename.empty())
	{
		throw std::runtime_error("ファイル名が空です");
	}

	// フルパス生成
	std::wstring fullPath = BuildShaderPath(filename);

	// ResourceManagerから取得を試みる
	auto& manager = ResourceManager::Instance();
	auto resource = manager.GetResource<PixelShader>(filename);
	
	if (resource)
	{
		// キャッシュされていたらそれを返す
		return resource;
	}
	
	// キャッシュされていない場合は新規作成
	auto pixelShader = std::make_shared<PixelShader>();
	
	// PixelShaderオブジェクトの初期化
	// 注: Initialize関数を使用して内部状態を設定します
	if (!pixelShader->Initialize(fullPath))
	{
		throw std::runtime_error("ピクセルシェーダーの初期化に失敗しました");
	}
	
	// ResourceManagerに登録
	manager.RegisterResource(filename, pixelShader);
	
	return pixelShader;
}
