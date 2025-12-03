/*============================================================================================================

    シェーダーファクトリー実装 [shader_factory.cpp]
    Shaderリソースの生成と管理を行うファクトリークラスの実装。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#include "resourcemanagement/shader_factory.h"
#include "resourcemanagement/shader.h"
#include "resourcemanagement/constant_buffer.h"
#include "resourcemanagement/resource_manager.h"
#include "direct3D/direct3d_device.h"
#include "rendering/render_system.h"
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <stdexcept>

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
	PerFrame定数バッファはDefaultタイプ、それ以外はDynamicタイプで作成する。
	
	引数:
	  blob - シェーダーのBlob
	  shader - 定数バッファ情報を格納するShaderオブジェクト
	例外: リフレクションに失敗した場合はruntime_errorをスロー
========================================================================================================*/
void ShaderFactory::ReflectShader(const Microsoft::WRL::ComPtr<ID3DBlob>& blob, Shader* shader)
{
	if (!blob || !shader)
	{
		throw std::runtime_error("ShaderFactory::ReflectShader - blob又はshaderがnullptrです");
	}
	
	// リフレクションインターフェースを作成
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflection;
	HRESULT hr = D3DReflect(
		blob.Get()->GetBufferPointer(),
		blob.Get()->GetBufferSize(),
		IID_PPV_ARGS(reflection.GetAddressOf())
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
	
	// 定数バッファ情報
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
		
		// バッファタイプを決定
		// "PerFrame"という名前の定数バッファはDefaultタイプ、それ以外はDynamicタイプ
		ConstantBufferUsage usage = ConstantBufferUsage::Dynamic;
		std::string bufferName = bufferDesc.Name;
		
		if (bufferName == "PerFrame")
		{
			usage = ConstantBufferUsage::Default;
		}
		
		// ConstantBufferクラスを作成
		auto constantBuffer = std::make_shared<ConstantBuffer>();
		constantBuffer->Initialize(bufferDesc.Name, bufferDesc.Size, usage);
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
	}
}

/*========================================================================================================
	ShaderFactory::GenerateInputLayoutFromReflection
	
	シェーダーリフレクションから入力レイアウトを自動生成する。
	シェーダーバイトコードからリフレクション情報を取得し、入力レイアウト記述を生成する。
	
	引数:
	  bytecode - シェーダーバイトコード
	戻り値: 生成された入力レイアウト記述の配列
	例外: 生成に失敗した場合はruntime_errorをスロー
========================================================================================================*/
std::vector<D3D11_INPUT_ELEMENT_DESC> ShaderFactory::GenerateInputLayoutFromReflection(
	const std::vector<uint8_t>& bytecode)
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout;

	if (bytecode.empty())
	{
		throw std::runtime_error("ShaderFactory::GenerateInputLayoutFromReflection - バイトコードが空です");
	}

	// シェーダーリフレクションインターフェースを作成
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflector;
	HRESULT hr = D3DReflect(
		bytecode.data(),
		bytecode.size(),
		IID_PPV_ARGS(pReflector.GetAddressOf())
	);

	if (FAILED(hr))
	{
		throw std::runtime_error("ShaderFactory::GenerateInputLayoutFromReflection - リフレクション情報の取得に失敗しました");
	}

	// シェーダー記述を取得
	D3D11_SHADER_DESC shaderDesc;
	hr = pReflector->GetDesc(&shaderDesc);
	
	if (FAILED(hr))
	{
		throw std::runtime_error("ShaderFactory::GenerateInputLayoutFromReflection - シェーダー記述の取得に失敗しました");
	}

	// セマンティック名を格納する文字列バッファ(D3D11_INPUT_ELEMENT_DESCのポインタを保持するため)
	static std::vector<std::string> semanticNames;
	semanticNames.clear();
	semanticNames.reserve(shaderDesc.InputParameters);

	// 各入力パラメータから入力レイアウトを生成
	for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		hr = pReflector->GetInputParameterDesc(i, &paramDesc);
		
		if (FAILED(hr))
		{
			throw std::runtime_error("ShaderFactory::GenerateInputLayoutFromReflection - 入力パラメータの取得に失敗しました");
		}

		// セマンティック名を保存
		semanticNames.push_back(paramDesc.SemanticName);

		// D3D11_INPUT_ELEMENT_DESCを生成
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = semanticNames.back().c_str();
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// マスクとコンポーネントタイプからフォーマットを決定
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		
		if (paramDesc.Mask == 1)
		{
			// 1コンポーネント
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				format = DXGI_FORMAT_R32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				format = DXGI_FORMAT_R32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				format = DXGI_FORMAT_R32_FLOAT;
			}
		}
		else if (paramDesc.Mask <= 3)
		{
			// 2コンポーネント
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				format = DXGI_FORMAT_R32G32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				format = DXGI_FORMAT_R32G32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				format = DXGI_FORMAT_R32G32_FLOAT;
			}
		}
		else if (paramDesc.Mask <= 7)
		{
			// 3コンポーネント
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				format = DXGI_FORMAT_R32G32B32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				format = DXGI_FORMAT_R32G32B32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
		}
		else if (paramDesc.Mask <= 15)
		{
			// 4コンポーネント
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				format = DXGI_FORMAT_R32G32B32A32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				format = DXGI_FORMAT_R32G32B32A32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
		}

		if (format == DXGI_FORMAT_UNKNOWN)
		{
			throw std::runtime_error("ShaderFactory::GenerateInputLayoutFromReflection - 未対応のフォーマットです");
		}

		elementDesc.Format = format;
		inputLayout.push_back(elementDesc);
	}

	return inputLayout;
}

/*========================================================================================================
	ShaderFactory::CreateVertexShader
	
	頂点シェーダーの生成または取得を行う。
	ResourceManagerにキャッシュされていればそれを返し、
	なければファイルから読み込んでResourceManagerに登録する。
	ファイルパスには自動的に"shaders/cso/"が先頭に付加される。
	作成したShaderはRenderSystemに登録される。
	
	引数:
	  filename - シェーダーファイルのパス（"shaders/cso/"は自動付加）
	戻り値: 頂点シェーダーのshared_ptr
	例外: 読み込みに失敗した場合はruntime_errorをスロー
========================================================================================================*/
std::shared_ptr<VertexShader> ShaderFactory::CreateVertexShader(
	const std::wstring& filename)
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

	// フルパス生成
	std::wstring fullPath = BuildShaderPath(filename);

	// ResourceManagerから取得を試行
	auto& manager = ResourceManager::Instance();
	auto resource = manager.GetResource<VertexShader>(filename);
	
	if (resource)
	{
		// キャッシュされていたらそれを返す
		return resource;
	}
	
	// キャッシュされていない場合は新規作成
	auto vertexShader = std::make_shared<VertexShader>();
	
	// シェーダーバイトコードを読み込む
	Microsoft::WRL::ComPtr<ID3DBlob> blob;
	LoadShaderFile(fullPath, blob);
	
	// バイトコードをvectorに変換
	std::vector<uint8_t> bytecode(
		static_cast<const uint8_t*>(blob->GetBufferPointer()),
		static_cast<const uint8_t*>(blob->GetBufferPointer()) + blob->GetBufferSize()
	);
	
	// リフレクションを使用して定数バッファ情報を取得
	ReflectShader(blob, vertexShader.get());
	
	// 頂点シェーダーを作成
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
	HRESULT hr = d3dDevice->CreateVertexShader(
		blob->GetBufferPointer(),
		blob->GetBufferSize(),
		nullptr,
		vs.GetAddressOf()
	);
	
	if (FAILED(hr))
	{
		throw std::runtime_error("頂点シェーダーの作成に失敗しました");
	}
	
	// リフレクションから入力レイアウトを自動生成
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc = GenerateInputLayoutFromReflection(bytecode);
	
	// 入力レイアウトを作成
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	hr = d3dDevice->CreateInputLayout(
		inputLayoutDesc.data(),
		static_cast<UINT>(inputLayoutDesc.size()),
		blob->GetBufferPointer(),
		blob->GetBufferSize(),
		inputLayout.GetAddressOf()
	);
	
	if (FAILED(hr))
	{
		throw std::runtime_error("入力レイアウトの作成に失敗しました");
	}
	
	// VertexShaderオブジェクトを初期化
	if (!vertexShader->Initialize(vs, inputLayout, bytecode))
	{
		throw std::runtime_error("頂点シェーダーの初期化に失敗しました");
	}
	
	// ResourceManagerに登録
	manager.RegisterResource(filename, vertexShader);
	
	// RenderSystemに登録
	RenderSystem::Instance().RegisterShader(vertexShader.get());
	
	return vertexShader;
}

/*========================================================================================================
	ShaderFactory::CreatePixelShader
	
	ピクセルシェーダーの作成又は取得を行う。
	ResourceManagerにキャッシュされていればそれを返し、
	なければファイルから読み込んでResourceManagerに登録する。
	ファイルパスには自動的に"shaders/cso/"を先頭に付加する。
	作成したShaderはRenderSystemに登録される。
	
	引数:
	  filename - シェーダーファイルのパス（"shaders/cso/"は自動付加）
	戻り値: ピクセルシェーダーのshared_ptr
	例外: 読み込みに失敗した場合はruntime_errorをスロー
========================================================================================================*/
std::shared_ptr<PixelShader> ShaderFactory::CreatePixelShader(
	const std::wstring& filename)
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

	// フルパス生成
	std::wstring fullPath = BuildShaderPath(filename);

	// ResourceManagerから取得を試行
	auto& manager = ResourceManager::Instance();
	auto resource = manager.GetResource<PixelShader>(filename);
	
	if (resource)
	{
		// キャッシュされていたらそれを返す
		return resource;
	}
	
	// キャッシュされていない場合は新規作成
	auto pixelShader = std::make_shared<PixelShader>();
	
	// シェーダーバイトコードを読み込む
	Microsoft::WRL::ComPtr<ID3DBlob> blob;
	LoadShaderFile(fullPath, blob);
	
	// バイトコードをvectorに変換
	std::vector<uint8_t> bytecode(
		static_cast<const uint8_t*>(blob->GetBufferPointer()),
		static_cast<const uint8_t*>(blob->GetBufferPointer()) + blob->GetBufferSize()
	);
	
	// リフレクションを使用して定数バッファ情報を取得
	ReflectShader(blob, pixelShader.get());
	
	// ピクセルシェーダーを作成
	Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
	HRESULT hr = d3dDevice->CreatePixelShader(
		blob->GetBufferPointer(),
		blob->GetBufferSize(),
		nullptr,
		ps.GetAddressOf()
	);
	
	if (FAILED(hr))
	{
		throw std::runtime_error("ピクセルシェーダーの作成に失敗しました");
	}
	
	// PixelShaderオブジェクトを初期化
	if (!pixelShader->Initialize(ps, bytecode))
	{
		throw std::runtime_error("ピクセルシェーダーの初期化に失敗しました");
	}
	
	// ResourceManagerに登録
	manager.RegisterResource(filename, pixelShader);
	
	// RenderSystemに登録
	RenderSystem::Instance().RegisterShader(pixelShader.get());
	
	return pixelShader;
}
