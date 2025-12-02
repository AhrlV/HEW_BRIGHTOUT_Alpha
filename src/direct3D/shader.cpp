/*============================================================================================================

    シェーダークラス実装 [shader.cpp]
    シェーダーの基底クラスと派生クラス（VertexShader、PixelShader）の実装。

    Author : Ryosuke Kageyama
    Date   : 2025/11/25

=============================================================================================================*/

#include "direct3D/shader.h"
#include "direct3D/constant_buffer.h"
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
    定数バッファを追加する
    指定されたサイズの定数バッファを作成し、内部リストに追加する。
    
    引数:
      byteWidth - 定数バッファのサイズ（バイト）
    戻り値: 追加された定数バッファのインデックス
    例外: 作成に失敗した場合はruntime_error例外をスロー
=============================================================================================================*/
size_t Shader::AddConstantBuffer(UINT byteWidth)
{
    // GraphicsDeviceから取得
    static auto& device = GraphicsDevice::Instance();
    static auto d3dDevice = device.GetDevice();

    if (!d3dDevice)
    {
        throw std::runtime_error("Shader::AddConstantBuffer - デバイスがnullptrです");
    }

    if (byteWidth == 0)
    {
        throw std::runtime_error("Shader::AddConstantBuffer - バッファサイズが0です");
    }

    if (byteWidth % 16 != 0)
    {
        throw std::runtime_error("Shader::AddConstantBuffer - バッファサイズは16バイトの倍数でなければなりません");
    }

    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = byteWidth;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;

    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
    HRESULT hr = d3dDevice->CreateBuffer(&bd, nullptr, buffer.GetAddressOf());

    if (FAILED(hr))
    {
        throw std::runtime_error("Shader::AddConstantBuffer - 定数バッファの作成に失敗しました");
    }

    m_ConstantBuffers.push_back(buffer);

    return m_ConstantBuffers.size() - 1;
}

/*============================================================================================================
    定数バッファを取得する
    指定されたインデックスの定数バッファを取得する。
    
    引数:
      index - 定数バッファのインデックス
    戻り値: 定数バッファのポインタ（存在しない場合はnullptr）
=============================================================================================================*/
ID3D11Buffer* Shader::GetConstantBuffer(size_t index) const
{
    if (index >= m_ConstantBuffers.size())
    {
        return nullptr;
    }

    return m_ConstantBuffers[index].Get();
}

/*============================================================================================================
    定数バッファ数を取得する
    戻り値: 定数バッファの総数
=============================================================================================================*/
size_t Shader::GetConstantBufferCount() const
{
    return m_ConstantBuffers.size();
}

/*============================================================================================================
    定数バッファを更新する
    指定されたインデックスの定数バッファにデータを書き込む。
    
    引数:
      index - 定数バッファのインデックス
      data - 更新するデータのポインタ
    例外: 更新に失敗した場合はruntime_errorをスロー
=============================================================================================================*/
void Shader::UpdateConstantBuffer(size_t index, const void* data)
{
    // GraphicsDeviceから取得
    static auto& device = GraphicsDevice::Instance();
    static auto context = device.GetContext();

    if (!context)
    {
        throw std::runtime_error("Shader::UpdateConstantBuffer - コンテキストがnullptrです");
    }

    if (!data)
    {
        throw std::runtime_error("Shader::UpdateConstantBuffer - データがnullptrです");
    }

    if (index >= m_ConstantBuffers.size())
    {
        throw std::runtime_error("Shader::UpdateConstantBuffer - インデックスが範囲外です");
    }

    context->UpdateSubresource(m_ConstantBuffers[index].Get(), 0, nullptr, data, 0, 0);
}

/*============================================================================================================
    定数バッファ情報を名前で取得する
    定数バッファマップから指定された名前のConstantBufferを取得する。
    
    引数:
      name - 定数バッファの名前
    戻り値: ConstantBufferのポインタ（存在しない場合はnullptr）
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
    リソースを解放する
    全てのシェーダーリソースを解放する。
=============================================================================================================*/
void Shader::Release()
{
    m_ConstantBuffers.clear();
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
    シェーダーファイル（.cso）を読み込む
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
        throw std::runtime_error("Shader::LoadShaderFile - ファイルを開けませんでした");
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
    頂点シェーダーとして初期化
    シェーダーファイル（.cso）を読み込み、頂点シェーダーと入力レイアウトを作成する。
    
    引数:
      filename - シェーダーファイルのパス
      inputLayout - 入力レイアウト記述の配列
    戻り値: 初期化に成功した場合true
    例外: 初期化に失敗した場合はruntime_error例外をスロー
=============================================================================================================*/
bool VertexShader::Initialize(
    const std::wstring& filename,
    const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayout)
{
    // GraphicsDeviceから取得
    static auto& device = GraphicsDevice::Instance();
    static auto d3dDevice = device.GetDevice();

    if (!d3dDevice)
    {
        throw std::runtime_error("VertexShader::Initialize - デバイスがnullptrです");
    }

    if (filename.empty())
    {
        throw std::runtime_error("VertexShader::Initialize - ファイル名が空です");
    }

    if (inputLayout.empty())
    {
        throw std::runtime_error("VertexShader::Initialize - 入力レイアウトが空です");
    }

    LoadShaderFile(filename, m_Bytecode);

    HRESULT hr = d3dDevice->CreateVertexShader(
        m_Bytecode.data(),
        m_Bytecode.size(),
        nullptr,
        m_VertexShader.GetAddressOf()
    );

    if (FAILED(hr))
    {
        throw std::runtime_error("VertexShader::Initialize - 頂点シェーダーの作成に失敗しました");
    }

    hr = d3dDevice->CreateInputLayout(
        inputLayout.data(),
        static_cast<UINT>(inputLayout.size()),
        m_Bytecode.data(),
        m_Bytecode.size(),
        m_InputLayout.GetAddressOf()
    );

    if (FAILED(hr))
    {
        throw std::runtime_error("VertexShader::Initialize - 入力レイアウトの作成に失敗しました");
    }

    return true;
}

/*============================================================================================================
    シェーダーをパイプラインに設定する
    頂点シェーダーと入力レイアウトをパイプラインに設定する。
    
    引数:
    例外: 設定に失敗した場合はruntime_error例外をスロー
=============================================================================================================*/
void VertexShader::SetToContext()
{
    // GraphicsDeviceから取得
    static auto& device = GraphicsDevice::Instance();
    static auto context = device.GetContext();

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
    // GraphicsDeviceから取得
    static auto& device = GraphicsDevice::Instance();
    static auto context = device.GetContext();

    if (!context)
    {
        throw std::runtime_error("VertexShader::SetConstantBuffers - コンテキストがnullptrです");
    }

    if (m_ConstantBuffers.empty())
    {
        return;
    }

    std::vector<ID3D11Buffer*> buffers;
    buffers.reserve(m_ConstantBuffers.size());
    for (const auto& cb : m_ConstantBuffers)
    {
        buffers.push_back(cb.Get());
    }

    context->VSSetConstantBuffers(startSlot, static_cast<UINT>(buffers.size()), buffers.data());
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
    シェーダーファイル（.cso）を読み込み、ピクセルシェーダーを作成する。
    
    引数:
      filename - シェーダーファイルのパス
    戻り値: 初期化に成功した場合true
    例外: 初期化に失敗した場合はruntime_error例外をスロー
=============================================================================================================*/
bool PixelShader::Initialize(
    const std::wstring& filename)
{
    // GraphicsDeviceから取得
    static auto& device = GraphicsDevice::Instance();
    static auto d3dDevice = device.GetDevice();

    if (!d3dDevice)
    {
        throw std::runtime_error("PixelShader::Initialize - デバイスがnullptrです");
    }

    if (filename.empty())
    {
        throw std::runtime_error("PixelShader::Initialize - ファイル名が空です");
    }

    LoadShaderFile(filename, m_Bytecode);

    HRESULT hr = d3dDevice->CreatePixelShader(
        m_Bytecode.data(),
        m_Bytecode.size(),
        nullptr,
        m_PixelShader.GetAddressOf()
    );

    if (FAILED(hr))
    {
        throw std::runtime_error("PixelShader::Initialize - ピクセルシェーダーの作成に失敗しました");
    }

    return true;
}

/*============================================================================================================
    シェーダーをパイプラインに設定する
    ピクセルシェーダーをパイプラインに設定する。
    
    例外: 設定に失敗した場合はruntime_error例外をスロー
=============================================================================================================*/
void PixelShader::SetToContext()
{
    // GraphicsDeviceから取得
    static auto& device = GraphicsDevice::Instance();
    static auto context = device.GetContext();

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
    // GraphicsDeviceから取得
    static auto& device = GraphicsDevice::Instance();
    static auto context = device.GetContext();

    if (!context)
    {
        throw std::runtime_error("PixelShader::SetConstantBuffers - コンテキストがnullptrです");
    }

    if (m_ConstantBuffers.empty())
    {
        return;
    }

    std::vector<ID3D11Buffer*> buffers;
    buffers.reserve(m_ConstantBuffers.size());
    for (const auto& cb : m_ConstantBuffers)
    {
        buffers.push_back(cb.Get());
    }

    context->PSSetConstantBuffers(startSlot, static_cast<UINT>(buffers.size()), buffers.data());
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
