/*============================================================================================================

    シェーダークラス実装 [shader.cpp]
    シェーダーの基底クラスと派生クラス(VertexShader、PixelShader)の実装。

    Author : Ryosuke Kageyama
    Date   : 2025/11/25

=============================================================================================================*/

#include "resourcemanagement/shader.h"
#include "resourcemanagement/constant_buffer.h"
#include <fstream>
#include <stdexcept>
#include "direct3D/direct3d_device.h"

/*============================================================================================================
    【Shader基底クラス】
=============================================================================================================*/

/*============================================================================================================
    デフォルトコンストラクタ
    メンバ変数を初期化する。
=============================================================================================================*/
Shader::Shader()
    : Resource(ResourceClassID::Shader)
    , m_ShaderType(ShaderType::Unknown)
{
}

/*============================================================================================================
    シェーダータイプを指定するコンストラクタ
    引数:
      type - シェーダーのタイプ
=============================================================================================================*/
Shader::Shader(ShaderType type)
    : Resource(ResourceClassID::Shader)
    , m_ShaderType(type)
{
}

/*============================================================================================================
    識別名とシェーダータイプを指定するコンストラクタ
    引数:
      name - このシェーダーの識別名
      type - シェーダーのタイプ
=============================================================================================================*/
Shader::Shader(const std::wstring& name, ShaderType type)
    : Resource(name, ResourceClassID::Shader)
    , m_ShaderType(type)
{
}

/*============================================================================================================
    デストラクタ
    リソースを解放する。
=============================================================================================================*/
Shader::~Shader()
{
    Release();
}

/*============================================================================================================
    定数バッファを名前で取得する
    定数バッファマップから指定された名前のConstantBufferを取得する。
    
    引数:
      name - 定数バッファの名前
    戻り値: ConstantBufferのポインタ(存在しない場合はnullptr)
=============================================================================================================*/
ConstantBuffer* Shader::GetConstantBufferByName(const std::string& name)
{
    auto it = m_ConstantBufferMap.find(name);
    if (it != m_ConstantBufferMap.end())
    {
        return it->second.get();
    }
    return nullptr;
}

/*============================================================================================================
    定数バッファ情報を追加する
    定数バッファマップに新しいConstantBufferを追加する。
    
    引数:
      name - 定数バッファの名前
      constantBuffer - ConstantBufferのshared_ptr
=============================================================================================================*/
void Shader::AddConstantBufferInfo(const std::string& name, std::shared_ptr<ConstantBuffer> constantBuffer)
{
    if (!constantBuffer)
    {
        throw std::runtime_error("Shader::AddConstantBufferInfo - constantBufferがnullptrです");
    }
    
    m_ConstantBufferMap[name] = constantBuffer;
}

/*============================================================================================================
    定数バッファに特定の変数が存在するか確認する
    指定された定数バッファに指定された変数が含まれているかチェックする。
    
    引数:
      bufferName - 定数バッファの名前
      variableName - 変数の名前
    戻り値: 変数が存在する場合true
=============================================================================================================*/
bool Shader::HasConstantBufferVariable(const std::string& bufferName, const std::string& variableName) const
{
    auto it = m_ConstantBufferMap.find(bufferName);
    if (it == m_ConstantBufferMap.end())
    {
        return false;
    }
    
    const ConstantBuffer* cb = it->second.get();
    if (!cb)
    {
        return false;
    }
    
    return cb->HasVariable(variableName);
}

/*============================================================================================================
    定数バッファを更新する(名前指定)
    指定された名前の定数バッファ全体を更新する。
    
    引数:
      bufferName - 定数バッファの名前
      data - 更新するデータのポインタ
    例外: 更新に失敗した場合はruntime_errorをスロー
=============================================================================================================*/
void Shader::UpdateConstantBuffer(const std::string& bufferName, const void* data)
{
    if (!data)
    {
        throw std::runtime_error("Shader::UpdateConstantBuffer - データがnullptrです");
    }
    
    ConstantBuffer* cb = GetConstantBufferByName(bufferName);
    if (!cb)
    {
        throw std::runtime_error("Shader::UpdateConstantBuffer - 定数バッファが見つかりません: " + bufferName);
    }
    
    cb->Update(data);
}

/*============================================================================================================
    定数バッファの変数を更新する(名前指定)
    指定された定数バッファ内の特定の変数のみを更新する。
    
    引数:
      bufferName - 定数バッファの名前
      variableName - 変数の名前
      data - 更新するデータのポインタ
      dataSize - データのサイズ(バイト)
    例外: 更新に失敗した場合はruntime_errorをスロー
=============================================================================================================*/
void Shader::UpdateConstantBufferVariable(const std::string& bufferName, const std::string& variableName, const void* data, UINT dataSize)
{
    if (!data)
    {
        throw std::runtime_error("Shader::UpdateConstantBufferVariable - データがnullptrです");
    }
    
    ConstantBuffer* cb = GetConstantBufferByName(bufferName);
    if (!cb)
    {
        throw std::runtime_error("Shader::UpdateConstantBufferVariable - 定数バッファが見つかりません: " + bufferName);
    }
    
    cb->UpdateVariable(variableName, data, dataSize);
}

/*============================================================================================================
    リソースを解放する
    全てのシェーダーリソースを解放する。
=============================================================================================================*/
void Shader::Release()
{
    m_ConstantBufferMap.clear();
    m_Bytecode.clear();
    m_ShaderType = ShaderType::Unknown;
}

/*============================================================================================================
    シェーダータイプを取得する
    戻り値: シェーダーのタイプ
=============================================================================================================*/
ShaderType Shader::GetShaderType() const
{
    return m_ShaderType;
}

/*============================================================================================================
    シェーダーバイトコードを取得する
    戻り値: シェーダーバイトコード
=============================================================================================================*/
const std::vector<uint8_t>& Shader::GetBytecode() const
{
    return m_Bytecode;
}

/*============================================================================================================
    シェーダーファイル(.cso)を読み込む
    コンパイル済みシェーダーファイルをバイナリとして読み込む。
    
    引数:
      filename - シェーダーファイルのパス
      bytecode - 読み込んだバイトコードを格納するベクター
    例外: 読み込みに失敗した場合はruntime_error例外をスロー
=============================================================================================================*/
void Shader::LoadShaderFile(const std::wstring& filename, std::vector<uint8_t>& bytecode)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        throw std::runtime_error("Shader::LoadShaderFile - ファイルが開けませんでした");
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size <= 0)
    {
        throw std::runtime_error("Shader::LoadShaderFile - ファイルが空です");
    }

    bytecode.resize(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(bytecode.data()), size))
    {
        throw std::runtime_error("Shader::LoadShaderFile - ファイルの読み込みに失敗しました");
    }

    file.close();
}

/*============================================================================================================
    【VertexShader派生クラス】
=============================================================================================================*/

/*============================================================================================================
    デフォルトコンストラクタ
    メンバ変数を初期化する。
=============================================================================================================*/
VertexShader::VertexShader()
    : Shader(ShaderType::Vertex)
    , m_VertexShader(nullptr)
    , m_InputLayout(nullptr)
{
}

/*============================================================================================================
    識別名を指定するコンストラクタ
    引数:
      name - この頂点シェーダーの識別名
=============================================================================================================*/
VertexShader::VertexShader(const std::wstring& name)
    : Shader(name, ShaderType::Vertex)
    , m_VertexShader(nullptr)
    , m_InputLayout(nullptr)
{
}

/*============================================================================================================
    デストラクタ
    リソースを解放する。
=============================================================================================================*/
VertexShader::~VertexShader()
{
    m_VertexShader.Reset();
    m_InputLayout.Reset();
}

/*============================================================================================================
    頂点シェーダーとして初期化(入力レイアウト指定版)
    外部から渡された頂点シェーダーと入力レイアウトを設定する。
    この関数はShaderFactoryから呼び出されることを想定している。
    
    引数:
      vertexShader - 作成済みの頂点シェーダー
      inputLayout - 作成済みの入力レイアウト
      bytecode - シェーダーバイトコード
    戻り値: 初期化に成功した場合true
    例外: 初期化に失敗した場合はruntime_error例外をスロー
=============================================================================================================*/
bool VertexShader::Initialize(
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader,
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout,
    const std::vector<uint8_t>& bytecode)
{
    if (!vertexShader)
    {
        throw std::runtime_error("VertexShader::Initialize - vertexShaderがnullptrです");
    }

    if (!inputLayout)
    {
        throw std::runtime_error("VertexShader::Initialize - inputLayoutがnullptrです");
    }

    if (bytecode.empty())
    {
        throw std::runtime_error("VertexShader::Initialize - バイトコードが空です");
    }

    m_VertexShader = vertexShader;
    m_InputLayout = inputLayout;
    m_Bytecode = bytecode;

    return true;
}

/*============================================================================================================
    シェーダーをパイプラインに設定する
    頂点シェーダーと入力レイアウトをパイプラインに設定する。
    
    例外: 設定に失敗した場合はruntime_error例外をスロー
=============================================================================================================*/
void VertexShader::SetToContext()
{
    auto& device = GraphicsDevice::Instance();
    auto context = device.GetContext();

    if (!context)
    {
        throw std::runtime_error("VertexShader::SetToContext - コンテキストがnullptrです");
    }

    if (!m_VertexShader)
    {
        throw std::runtime_error("VertexShader::SetToContext - 頂点シェーダーが無効です");
    }

    context->VSSetShader(m_VertexShader.Get(), nullptr, 0);
    
    if (m_InputLayout)
    {
        context->IASetInputLayout(m_InputLayout.Get());
    }
}

/*============================================================================================================
    定数バッファをシェーダーに設定する
    定数バッファを頂点シェーダーに設定する。
    
    引数:
      startSlot - 開始スロット番号
=============================================================================================================*/
void VertexShader::SetConstantBuffers(UINT startSlot)
{
    auto& device = GraphicsDevice::Instance();
    auto context = device.GetContext();

    if (!context)
    {
        throw std::runtime_error("VertexShader::SetConstantBuffers - コンテキストがnullptrです");
    }

    if (m_ConstantBufferMap.empty())
    {
        return;
    }

    std::vector<ID3D11Buffer*> buffers;
    for (const auto& pair : m_ConstantBufferMap)
    {
        if (pair.second && pair.second->GetBuffer())
        {
            buffers.push_back(pair.second->GetBuffer());
        }
    }

    if (!buffers.empty())
    {
        context->VSSetConstantBuffers(startSlot, static_cast<UINT>(buffers.size()), buffers.data());
    }
}

/*============================================================================================================
    リソースが有効かどうかを判定する
    戻り値: 頂点シェーダーが有効な場合true
=============================================================================================================*/
bool VertexShader::IsValid() const
{
    return m_VertexShader.Get() != nullptr;
}

/*============================================================================================================
    頂点シェーダーを取得する
    戻り値: 頂点シェーダーのポインタ
=============================================================================================================*/
ID3D11VertexShader* VertexShader::GetVertexShader() const
{
    return m_VertexShader.Get();
}

/*============================================================================================================
    入力レイアウトを取得する
    戻り値: 入力レイアウトのポインタ
=============================================================================================================*/
ID3D11InputLayout* VertexShader::GetInputLayout() const
{
    return m_InputLayout.Get();
}

/*============================================================================================================
    【PixelShader派生クラス】
=============================================================================================================*/

/*============================================================================================================
    デフォルトコンストラクタ
    メンバ変数を初期化する。
=============================================================================================================*/
PixelShader::PixelShader()
    : Shader(ShaderType::Pixel)
    , m_PixelShader(nullptr)
{
}

/*============================================================================================================
    識別名を指定するコンストラクタ
    引数:
      name - このピクセルシェーダーの識別名
=============================================================================================================*/
PixelShader::PixelShader(const std::wstring& name)
    : Shader(name, ShaderType::Pixel)
    , m_PixelShader(nullptr)
{
}

/*============================================================================================================
    デストラクタ
    リソースを解放する。
=============================================================================================================*/
PixelShader::~PixelShader()
{
    m_PixelShader.Reset();
}

/*============================================================================================================
    ピクセルシェーダーとして初期化
    外部から渡されたピクセルシェーダーを設定する。
    この関数はShaderFactoryから呼び出されることを想定している。
    
    引数:
      pixelShader - 作成済みのピクセルシェーダー
      bytecode - シェーダーバイトコード
    戻り値: 初期化に成功した場合true
    例外: 初期化に失敗した場合はruntime_error例外をスロー
=============================================================================================================*/
bool PixelShader::Initialize(
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader,
    const std::vector<uint8_t>& bytecode)
{
    if (!pixelShader)
    {
        throw std::runtime_error("PixelShader::Initialize - pixelShaderがnullptrです");
    }

    if (bytecode.empty())
    {
        throw std::runtime_error("PixelShader::Initialize - バイトコードが空です");
    }

    m_PixelShader = pixelShader;
    m_Bytecode = bytecode;

    return true;
}

/*============================================================================================================
    シェーダーをパイプラインに設定する
    ピクセルシェーダーをパイプラインに設定する。
    
    例外: 設定に失敗した場合はruntime_error例外をスロー
=============================================================================================================*/
void PixelShader::SetToContext()
{
    auto& device = GraphicsDevice::Instance();
    auto context = device.GetContext();

    if (!context)
    {
        throw std::runtime_error("PixelShader::SetToContext - コンテキストがnullptrです");
    }

    if (!m_PixelShader)
    {
        throw std::runtime_error("PixelShader::SetToContext - ピクセルシェーダーが無効です");
    }

    context->PSSetShader(m_PixelShader.Get(), nullptr, 0);
}

/*============================================================================================================
    定数バッファをシェーダーに設定する
    定数バッファをピクセルシェーダーに設定する。
    
    引数:
      startSlot - 開始スロット番号
=============================================================================================================*/
void PixelShader::SetConstantBuffers(UINT startSlot)
{
    auto& device = GraphicsDevice::Instance();
    auto context = device.GetContext();

    if (!context)
    {
        throw std::runtime_error("PixelShader::SetConstantBuffers - コンテキストがnullptrです");
    }

    if (m_ConstantBufferMap.empty())
    {
        return;
    }

    std::vector<ID3D11Buffer*> buffers;
    for (const auto& pair : m_ConstantBufferMap)
    {
        if (pair.second && pair.second->GetBuffer())
        {
            buffers.push_back(pair.second->GetBuffer());
        }
    }

    if (!buffers.empty())
    {
        context->PSSetConstantBuffers(startSlot, static_cast<UINT>(buffers.size()), buffers.data());
    }
}

/*============================================================================================================
    リソースが有効かどうかを判定する
    戻り値: ピクセルシェーダーが有効な場合true
=============================================================================================================*/
bool PixelShader::IsValid() const
{
    return m_PixelShader.Get() != nullptr;
}

/*============================================================================================================
    ピクセルシェーダーを取得する
    戻り値: ピクセルシェーダーのポインタ
=============================================================================================================*/
ID3D11PixelShader* PixelShader::GetPixelShader() const
{
    return m_PixelShader.Get();
}
