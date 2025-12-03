/*============================================================================================================

    メッシュクラス実装 [mesh.cpp]
    3Dモデルの頂点データとインデックスデータの管理を行う。

    Author : Ryosuke Kageyama
    Date   : 2025/05/07
    Date   : 2025/11/25

=============================================================================================================*/

#include "resourcemanagement/mesh.h"
#include "resourcemanagement/shader.h"
#include "direct3D/direct3d_device.h"
#include "resourcemanagement/shader_factory.h"
#include <stdexcept>

/*============================================================================================================
    デフォルトコンストラクタ
    メンバ変数を初期化する。
=============================================================================================================*/
Mesh::Mesh()
    : Resource(ResourceClassID::Mesh)
    , m_VertexBuffer(nullptr)
    , m_IndexBuffer(nullptr)
    , m_IndexCount(0)
    , m_Topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
    , m_VertexShader(nullptr)
{
}

/*============================================================================================================
    識別名を指定するコンストラクタ
    引数:
      name - このメッシュの識別名
=============================================================================================================*/
Mesh::Mesh(const std::wstring& name)
    : Resource(name, ResourceClassID::Mesh)
    , m_VertexBuffer(nullptr)
    , m_IndexBuffer(nullptr)
    , m_IndexCount(0)
    , m_Topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
    , m_VertexShader(nullptr)
{
}

/*============================================================================================================
    デストラクタ
    リソースを解放する。
=============================================================================================================*/
Mesh::~Mesh()
{
    Release();
}

/*============================================================================================================
    メッシュの初期化
    頂点データとインデックスデータから頂点バッファとインデックスバッファを作成する。
    
    引数:
      vertices - 頂点データの配列
      indices - インデックスデータの配列
    戻り値: 初期化に成功した場合true
    例外: 初期化に失敗した場合はruntime_error例外をスロー
=============================================================================================================*/
bool Mesh::Initialize(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices)
{
    // GraphicsDeviceから取得
    static auto& device = GraphicsDevice::Instance();
    static auto d3dDevice = device.GetDevice();

    if (!d3dDevice)
    {
        throw std::runtime_error("Mesh::Initialize - デバイスがnullptrです");
    }

    if (vertices.empty())
    {
        throw std::runtime_error("Mesh::Initialize - 頂点データが空です");
    }

    if (indices.empty())
    {
        throw std::runtime_error("Mesh::Initialize - インデックスデータが空です");
    }

    m_IndexCount = static_cast<UINT>(indices.size());

    D3D11_BUFFER_DESC vbd{};
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(MeshVertex));
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vinit{};
    vinit.pSysMem = vertices.data();

    HRESULT hr = d3dDevice->CreateBuffer(&vbd, &vinit, m_VertexBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        throw std::runtime_error("Mesh::Initialize - 頂点バッファの作成に失敗しました");
    }

    D3D11_BUFFER_DESC ibd{};
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.ByteWidth = static_cast<UINT>(indices.size() * sizeof(uint32_t));
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA iinit{};
    iinit.pSysMem = indices.data();

    hr = d3dDevice->CreateBuffer(&ibd, &iinit, m_IndexBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        throw std::runtime_error("Mesh::Initialize - インデックスバッファの作成に失敗しました");
    }

    m_VertexShader = ShaderFactory::CreateVertexShader(L"VS_3D_Default.cso");

    return true;
}

/*============================================================================================================
    リソースを解放する
    頂点バッファとインデックスバッファを解放し、データをリセットする。
=============================================================================================================*/
void Mesh::Release()
{
    m_VertexBuffer.Reset();
    m_IndexBuffer.Reset();
    m_IndexCount = 0;
    m_VertexShader.reset();
}

/*============================================================================================================
    リソースが有効かどうかを判定する
    戻り値: 頂点バッファとインデックスバッファが有効な場合true
=============================================================================================================*/
bool Mesh::IsValid() const
{
    return m_VertexBuffer.Get() != nullptr && m_IndexBuffer.Get() != nullptr;
}

/*============================================================================================================
    頂点バッファを取得する
    戻り値: 頂点バッファのポインタ
=============================================================================================================*/
ID3D11Buffer* Mesh::GetVertexBuffer() const
{
    return m_VertexBuffer.Get();
}

/*============================================================================================================
    インデックスバッファを取得する
    戻り値: インデックスバッファのポインタ
=============================================================================================================*/
ID3D11Buffer* Mesh::GetIndexBuffer() const
{
    return m_IndexBuffer.Get();
}

/*============================================================================================================
    インデックス数を取得する
    戻り値: インデックスの総数
=============================================================================================================*/
UINT Mesh::GetIndexCount() const
{
    return m_IndexCount;
}

/*============================================================================================================
    プリミティブトポロジーを取得する
    戻り値: プリミティブトポロジー（三角形リストなど）
=============================================================================================================*/
D3D11_PRIMITIVE_TOPOLOGY Mesh::GetTopology() const
{
    return m_Topology;
}

/*============================================================================================================
    頂点シェーダーを取得する
    戻り値: 頂点シェーダーのshared_ptr
=============================================================================================================*/
std::shared_ptr<VertexShader> Mesh::GetVertexShader() const
{
    return m_VertexShader;
}

/*============================================================================================================
    プリミティブトポロジーを設定する
    引数:
      topo - 設定するプリミティブトポロジー
=============================================================================================================*/
void Mesh::SetTopology(D3D11_PRIMITIVE_TOPOLOGY topo)
{
    m_Topology = topo;
}

/*============================================================================================================
    頂点シェーダーを設定する
    引数:
      vs - 設定する頂点シェーダーのshared_ptr
=============================================================================================================*/
void Mesh::SetVertexShader(std::shared_ptr<VertexShader> vs)
{
    m_VertexShader = vs;
}

/*============================================================================================================
    頂点シェーダーのPerObject定数バッファを更新する
    ワールド行列をシェーダーに送信する。
    
    引数:
      worldMatrix - ワールド変換行列
      vertexShader - 更新対象の頂点シェーダー
    例外: 更新に失敗した場合はruntime_errorをスロー
=============================================================================================================*/
void Mesh::UpdateVertexShaderPerObject(const DirectX::XMMATRIX& worldMatrix, VertexShader* vertexShader) const
{
    if (!vertexShader)
    {
        throw std::runtime_error("Mesh::UpdateVertexShaderPerObject - vertexShaderがnullptrです");
    }
    
    using namespace DirectX;
    
    // 転置（HLSLは列優先、C++は行優先）
    XMMATRIX worldTranspose = XMMatrixTranspose(worldMatrix);
    
    // PerObject定数バッファに格納するデータ構造
    struct PerObjectData
    {
        XMFLOAT4X4 world;
    };
    
    PerObjectData data;
    XMStoreFloat4x4(&data.world, worldTranspose);
    
    // "PerObject"定数バッファを更新
    vertexShader->UpdateConstantBuffer("PerObject", &data);
}
