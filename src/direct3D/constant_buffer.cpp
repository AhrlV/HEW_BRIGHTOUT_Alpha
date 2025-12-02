/*============================================================================================================

    定数バッファクラス実装 [constant_buffer.cpp]
    シェーダーの定数バッファ情報を管理するクラスの実装。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#include "direct3D/constant_buffer.h"
#include "direct3D/direct3d_device.h"
#include <stdexcept>

/*========================================================================================================
    デフォルトコンストラクタ
    メンバ変数を初期化する。
========================================================================================================*/
ConstantBuffer::ConstantBuffer()
    : m_Name("")
    , m_Size(0)
    , m_BindSlot(0)
    , m_Buffer(nullptr)
{
}

/*========================================================================================================
    バッファ名とサイズを指定するコンストラクタ
    引数:
      name - バッファの名前
      size - バッファのサイズ（バイト）
========================================================================================================*/
ConstantBuffer::ConstantBuffer(const std::string& name, UINT size)
    : m_Name(name)
    , m_Size(size)
    , m_BindSlot(0)
    , m_Buffer(nullptr)
{
    Initialize(name, size);
}

/*========================================================================================================
    デストラクタ
    リソースを解放する。
========================================================================================================*/
ConstantBuffer::~ConstantBuffer()
{
    Release();
}

/*========================================================================================================
    定数バッファを初期化する
    Direct3Dデバイスを使用して定数バッファを作成する。
    
    引数:
      name - バッファの名前
      size - バッファのサイズ（バイト）
    戻り値: 初期化に成功した場合true
    例外: 初期化に失敗した場合はruntime_errorをスロー
========================================================================================================*/
bool ConstantBuffer::Initialize(const std::string& name, UINT size)
{
    // GraphicsDeviceから取得
    auto& device = GraphicsDevice::Instance();
    auto d3dDevice = device.GetDevice();
    
    if (!d3dDevice)
    {
        throw std::runtime_error("ConstantBuffer::Initialize - デバイスがnullptrです");
    }
    
    if (name.empty())
    {
        throw std::runtime_error("ConstantBuffer::Initialize - バッファ名が空です");
    }
    
    if (size == 0)
    {
        throw std::runtime_error("ConstantBuffer::Initialize - バッファサイズが0です");
    }
    
    if (size % 16 != 0)
    {
        throw std::runtime_error("ConstantBuffer::Initialize - バッファサイズは16バイトの倍数でなければなりません");
    }
    
    m_Name = name;
    m_Size = size;
    
    // バッファ記述子を設定
    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = size;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;
    
    HRESULT hr = d3dDevice->CreateBuffer(&bd, nullptr, m_Buffer.GetAddressOf());
    
    if (FAILED(hr))
    {
        throw std::runtime_error("ConstantBuffer::Initialize - 定数バッファの作成に失敗しました");
    }
    
    return true;
}

/*========================================================================================================
    定数バッファを解放する
    バッファオブジェクトと変数情報をクリアする。
========================================================================================================*/
void ConstantBuffer::Release()
{
    m_Buffer.Reset();
    m_Variables.clear();
    m_Name.clear();
    m_Size = 0;
    m_BindSlot = 0;
}

/*========================================================================================================
    変数情報を追加する
    バッファ内の変数名とオフセット値を登録する。
    
    引数:
      variableName - 変数の名前
      offset - バッファ内でのオフセット値（バイト）
========================================================================================================*/
void ConstantBuffer::AddVariable(const std::string& variableName, UINT offset)
{
    m_Variables[variableName] = offset;
}

/*========================================================================================================
    変数のオフセット値を取得する
    登録されている変数のオフセット値を取得する。
    
    引数:
      variableName - 変数の名前
    戻り値: オフセット値（変数が存在しない場合は0）
========================================================================================================*/
UINT ConstantBuffer::GetVariableOffset(const std::string& variableName) const
{
    auto it = m_Variables.find(variableName);
    if (it != m_Variables.end())
    {
        return it->second;
    }
    return 0;
}

/*========================================================================================================
    変数が存在するか確認する
    指定された変数名が登録されているか確認する。
    
    引数:
      variableName - 変数の名前
    戻り値: 変数が存在する場合true
========================================================================================================*/
bool ConstantBuffer::HasVariable(const std::string& variableName) const
{
    return m_Variables.find(variableName) != m_Variables.end();
}

/*========================================================================================================
    バッファ名を取得する
    戻り値: バッファの名前
========================================================================================================*/
const std::string& ConstantBuffer::GetName() const
{
    return m_Name;
}

/*========================================================================================================
    バッファサイズを取得する
    戻り値: バッファのサイズ（バイト）
========================================================================================================*/
UINT ConstantBuffer::GetSize() const
{
    return m_Size;
}

/*========================================================================================================
    バッファオブジェクトを取得する
    戻り値: ID3D11Bufferのポインタ
========================================================================================================*/
ID3D11Buffer* ConstantBuffer::GetBuffer() const
{
    return m_Buffer.Get();
}

/*========================================================================================================
    バインドスロット番号を取得する
    戻り値: バインドスロット番号
========================================================================================================*/
UINT ConstantBuffer::GetBindSlot() const
{
    return m_BindSlot;
}

/*========================================================================================================
    バインドスロット番号を設定する
    引数:
      slot - バインドスロット番号
========================================================================================================*/
void ConstantBuffer::SetBindSlot(UINT slot)
{
    m_BindSlot = slot;
}

/*========================================================================================================
    バッファを更新する
    定数バッファの内容を更新する。
    
    引数:
      data - 更新するデータのポインタ
    例外: 更新に失敗した場合はruntime_errorをスロー
========================================================================================================*/
void ConstantBuffer::Update(const void* data)
{
    // GraphicsDeviceから取得
    auto& device = GraphicsDevice::Instance();
    auto context = device.GetContext();
    
    if (!context)
    {
        throw std::runtime_error("ConstantBuffer::Update - コンテキストがnullptrです");
    }
    
    if (!data)
    {
        throw std::runtime_error("ConstantBuffer::Update - データがnullptrです");
    }
    
    if (!m_Buffer)
    {
        throw std::runtime_error("ConstantBuffer::Update - バッファが作成されていません");
    }
    
    context->UpdateSubresource(m_Buffer.Get(), 0, nullptr, data, 0, 0);
}
