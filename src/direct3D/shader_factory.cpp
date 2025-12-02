/*============================================================================================================

    シェーダーファクトリー実装 [shader_factory.cpp]
    Shaderリソースの生成を管理するファクトリークラスの実装。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#include "direct3D/shader_factory.h"
#include "direct3D/shader.h"
#include "direct3D/constant_buffer.h"
#include "direct3D/resource_manager.h"
#include "direct3D/direct3d_device.h"
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <stdexcept>
#include <fstream>

#pragma comment(lib, "d3dcompiler.lib")

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
	
	シェーダーファイル(.cso)をID3D11Blobを使用して読み込む。
	コンパイル済みシェーダーファイルをバイナリとして読み込む。
	
	引数:
	  filename - シェーダーファイルのパス
	  blob - 読み込んだBlobを格納するComPtr
	例外: 読み込みに失敗した場合はruntime_errorをスロー
========================================================================================================*/
void ShaderFactory::LoadShaderFile(const std::wstring& filename, Microsoft::WRL::ComPtr<ID3DBlob>& blob)
{
	HRESULT hr = D3DReadFileToBlob(filename.c_str(), blob.GetAddressOf());
	
	if (FAILED(hr))
	{
		throw std::runtime_error("ShaderFactory::LoadShaderFile - ファイルの読み込みに失敗しました");
	}
	
	if (!blob || blob.Get()->GetBufferSize() == 0)
	{
		throw std::runtime_error("ShaderFactory::LoadShaderFile - ファイルが空です");
	}
}

/*========================================================================================================
	ShaderFactory::ReflectShader
	
	シェーダーのリフレクション情報を取得し、定数バッファ情報を抽出する。
	
	引数:
	  blob - シェーダーのBlob
	  shader - 定数バッファ情報を格納するShaderオブジェクト
	例外: リフレクションに失敗した場合はruntime_errorをスロー
========================================================================================================*/
void ShaderFactory::ReflectShader(const Microsoft::WRL::ComPtr<ID3DBlob>& blob, Shader* shader)
{
	if (!blob || !shader)
	{
		throw std::runtime_error("ShaderFactory::ReflectShader - blobまたはshaderがnullptrです");
	}
	
	// リフレクションインターフェースを作成
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflection;
	HRESULT hr = D3DReflect(
		blob.Get()->GetBufferPointer(),
		blob.Get()->GetBufferSize(),
		IID_ID3D11ShaderReflection,
		reinterpret_cast<void**>(reflection.GetAddressOf())
	);
	
	if (FAILED(hr))
	{
		throw std::runtime_error("ShaderFactory::ReflectShader - リフレクションの作成に失敗しました");
	}
	
	// シェーダー記述を取得
	D3D11_SHADER_DESC shaderDesc{};
	hr = reflection->GetDesc(&shaderDesc);
	
	if (FAILED(hr))
	{
		throw std::runtime_error("ShaderFactory::ReflectShader - シェーダー記述の取得に失敗しました");
	}
	
	// 定数バッファを解析
	for (UINT i = 0; i < shaderDesc.ConstantBuffers; ++i)
	{
		ID3D11ShaderReflectionConstantBuffer* cbReflection = reflection->GetConstantBufferByIndex(i);
		
		if (!cbReflection)
		{
			continue;
		}
		
		// 定数バッファ記述を取得
		D3D11_SHADER_BUFFER_DESC bufferDesc{};
		hr = cbReflection->GetDesc(&bufferDesc);
		
		if (FAILED(hr))
		{
			continue;
		}
		
		// ConstantBufferクラスを作成
		auto constantBuffer = std::make_shared<ConstantBuffer>();
		constantBuffer->Initialize(bufferDesc.Name, bufferDesc.Size);
		constantBuffer->SetBindSlot(i);
		
		// 変数情報を取得
		for (UINT j = 0; j < bufferDesc.Variables; ++j)
		{
			ID3D11ShaderReflectionVariable* varReflection = cbReflection->GetVariableByIndex(j);
			
			if (!varReflection)
			{
				continue;
			}
			
			D3D11_SHADER_VARIABLE_DESC varDesc{};
			hr = varReflection->GetDesc(&varDesc);
			
			if (FAILED(hr))
			{
				continue;
			}
			
			// 変数情報を追加
			constantBuffer->AddVariable(varDesc.Name, varDesc.StartOffset);
		}
		
		// Shaderクラスに定数バッファ情報を追加
		shader->AddConstantBufferInfo(bufferDesc.Name, constantBuffer);
		
		// 古い方式の定数バッファも追加（互換性のため）
		shader->AddConstantBuffer(bufferDesc.Size);
	}
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
	Microsoft::WRL::ComPtr<ID3DBlob> blob;
	LoadShaderFile(fullPath, blob);
	
	// リフレクションを使用して定数バッファ情報を取得
	ReflectShader(blob, vertexShader.get());
	
	// 頂点シェーダーを作成
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
	HRESULT hr = d3dDevice->CreateVertexShader(
		blob.Get()->GetBufferPointer(),
		blob.Get()->GetBufferSize(),
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
		blob.Get()->GetBufferPointer(),
		blob.Get()->GetBufferSize(),
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
	
	// シェーダーバイトコードを読み込み
	Microsoft::WRL::ComPtr<ID3DBlob> blob;
	LoadShaderFile(fullPath, blob);
	
	// リフレクションを使用して定数バッファ情報を取得
	ReflectShader(blob, pixelShader.get());
	
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
