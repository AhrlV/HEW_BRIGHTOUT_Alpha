/*============================================================================================================

    メッシュレンダラークラス実装 [mesh_renderer.cpp]
    メッシュとマテリアルを使用して3Dモデルを描画する。

    Author : Ryosuke Kageyama
    Date   : 2025/11/25

=============================================================================================================*/

#include "direct3D/mesh_renderer.h"
#include "direct3D/direct3d_device.h"
#include <stdexcept>
#include <DirectXMath.h>
#include "direct3D/resource_factory.h"

using namespace DirectX;

/*============================================================================================================
    デフォルトコンストラクタ
    メンバ変数を初期化する。
=============================================================================================================*/
MeshRenderer::MeshRenderer()
    : m_Mesh(nullptr)
    , m_Material(nullptr)
{
    m_Material = MaterialFactory::Create(L"default.mat");
}

/*============================================================================================================
    デストラクタ
    リソースを解放する。
=============================================================================================================*/
MeshRenderer::~MeshRenderer()
{
    m_Mesh.reset();
    m_Material.reset();
}

/*============================================================================================================
    コンポーネント初期化
    TransFormコンポーネントの参照を取得する。
=============================================================================================================*/
void MeshRenderer::Awake()
{
    if (!m_Owner)
    {
        throw std::runtime_error("MeshRenderer::Awake - オーナーGameObjectがnullptrです");
    }
}

/*============================================================================================================
    描画処理
    メッシュとマテリアルを使用して3Dモデルを描画する。
    頂点シェーダーはメッシュから、ピクセルシェーダーはマテリアルから取得する。
    
    引数:
      camera - カメラの参照
=============================================================================================================*/
void MeshRenderer::Render(const Camera& camera)
{
    // 必須項目チェック
    if (!m_Mesh)
    {
        throw std::runtime_error("MeshRenderer::Render - メッシュが設定されていません");
    }

    if (!m_Material)
    {
        throw std::runtime_error("MeshRenderer::Render - マテリアルが設定されていません");
    }

    if (!m_Owner)
    {
        throw std::runtime_error("MeshRenderer::Render - オーナーGameObjectがnullptrです");
    }

    // 頂点シェーダーの取得と設定（メッシュから）
    auto vertexShader = m_Mesh->GetVertexShader();
    if (vertexShader)
    {
        SetupVertexShader(vertexShader.get(), camera);
    }

    // ピクセルシェーダーの取得と設定（マテリアルから）
    auto pixelShader = m_Material->GetPixelShader();
    if (pixelShader)
    {
        SetupPixelShader(pixelShader.get());
    }

    // レンダリングステートの設定
    SetupRenderStates();

    // メッシュの描画
    DrawMesh();
}

/*============================================================================================================
    頂点シェーダーを設定する
    頂点シェーダーをパイプラインに設定し、定数バッファ（World/View/Projection行列）を更新する。
    
    引数:
      vertexShader - 頂点シェーダーのポインタ
      camera - カメラの参照
=============================================================================================================*/
void MeshRenderer::SetupVertexShader(VertexShader* vertexShader, const Camera& camera)
{
    if (!vertexShader || !vertexShader->IsValid())
    {
        return;
    }

    // 頂点シェーダーと入力レイアウトをパイプラインに設定
    vertexShader->SetToContext();

    // 定数バッファが3個以上ある場合、World/View/Projection行列を設定
    if (vertexShader->GetConstantBufferCount() >= 3)
    {
        auto tf = m_Owner->GetComponent<TransForm>();

        // World行列（転置して設定）
        XMFLOAT4X4 worldMatrix;
        XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(tf->GetWorldMatrix()));
        vertexShader->UpdateConstantBuffer(0, &worldMatrix);

        // View行列（転置して設定）
        XMFLOAT4X4 viewMatrix;
        XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(camera.GetViewMatrix()));
        vertexShader->UpdateConstantBuffer(1, &viewMatrix);

        // Projection行列（転置して設定）
        XMFLOAT4X4 projMatrix;
        XMStoreFloat4x4(&projMatrix, XMMatrixTranspose(camera.GetProjMatrix()));
        vertexShader->UpdateConstantBuffer(2, &projMatrix);

        // 定数バッファをシェーダーに設定
        vertexShader->SetConstantBuffers(0);
    }
}

/*============================================================================================================
    ピクセルシェーダーを設定する
    ピクセルシェーダーをパイプラインに設定し、定数バッファ（ライティング、マテリアル色）を更新する。
    
    引数:
      pixelShader - ピクセルシェーダーのポインタ
=============================================================================================================*/
void MeshRenderer::SetupPixelShader(PixelShader* pixelShader)
{
    if (!pixelShader || !pixelShader->IsValid())
    {
        return;
    }

    // GraphicsDeviceからDeviceContextを取得
    auto& device = GraphicsDevice::Instance();
    auto context = device.GetContext();

    // ピクセルシェーダーをパイプラインに設定
    pixelShader->SetToContext();

    // 定数バッファが3個以上ある場合、ライティング情報とマテリアル色を設定
    if (pixelShader->GetConstantBufferCount() >= 3)
    {

        // アンビエントライト（環境光）
        XMFLOAT4 ambientLight(1.0f, 1.0f, 1.0f, 1.0f);
        pixelShader->UpdateConstantBuffer(0, &ambientLight);

        // ディフューズライト（拡散光）とライト方向
        struct DiffuseLightPack
        {
            XMFLOAT4 color;
            XMFLOAT4 direction;
        };

        DiffuseLightPack diffuseLight;
        diffuseLight.color = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
        diffuseLight.direction = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
        pixelShader->UpdateConstantBuffer(1, &diffuseLight);

        // マテリアルカラー
        XMFLOAT4 materialColor = m_Material->Color;
        pixelShader->UpdateConstantBuffer(2, &materialColor);

        // 定数バッファをシェーダーに設定
        pixelShader->SetConstantBuffers(0);
    }

    // テクスチャの設定
    if (m_Material->Texture)
    {
        ID3D11ShaderResourceView* srv = m_Material->Texture.get()->GetShaderResourceView();
        context->PSSetShaderResources(0, 1, &srv);
    }

    // サンプラーステートの設定
    ID3D11SamplerState* sampler = m_Material->GetSampler();
    if (sampler)
    {
        context->PSSetSamplers(0, 1, &sampler);
    }
}

/*============================================================================================================
    レンダリングステートを設定する
    ブレンドステートと深度ステンシルステートをパイプラインに設定する。
=============================================================================================================*/
void MeshRenderer::SetupRenderStates()
{
    if (!m_Material)
    {
        return;
    }

    // GraphicsDeviceからDeviceContextを取得
    auto& device = GraphicsDevice::Instance();
    auto context = device.GetContext();

    // ブレンドステートの設定
    ID3D11BlendState* blendState = m_Material->GetBlendState();
    if (blendState)
    {
        float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        context->OMSetBlendState(blendState, blendFactor, 0xFFFFFFFF);
    }

    // 深度ステンシルステートの設定
    ID3D11DepthStencilState* depthState = m_Material->GetDepthState();
    if (depthState)
    {
        context->OMSetDepthStencilState(depthState, 0);
    }
}

/*============================================================================================================
    メッシュバッファを設定して描画する
    頂点バッファとインデックスバッファをパイプラインに設定し、DrawIndexedを呼び出す。
=============================================================================================================*/
void MeshRenderer::DrawMesh()
{
    if (!m_Mesh)
    {
        return;
    }

    // GraphicsDeviceからDeviceContextを取得
    auto& device = GraphicsDevice::Instance();
    auto context = device.GetContext();

    // 頂点バッファの取得
    ID3D11Buffer* vertexBuffer = m_Mesh->GetVertexBuffer();
    if (!vertexBuffer)
    {
        return;
    }

    // インデックスバッファの取得
    ID3D11Buffer* indexBuffer = m_Mesh->GetIndexBuffer();
    if (!indexBuffer)
    {
        return;
    }

    // 頂点バッファの設定
    UINT stride = sizeof(MeshVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    // インデックスバッファの設定
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // プリミティブトポロジーの設定
    context->IASetPrimitiveTopology(m_Mesh->GetTopology());

    // インデックス描画
    context->DrawIndexed(m_Mesh->GetIndexCount(), 0, 0);
}

/*============================================================================================================
    メッシュを設定する
    引数:
      mesh - 設定するメッシュのshared_ptr
=============================================================================================================*/
void MeshRenderer::SetMesh(std::shared_ptr<Mesh> mesh)
{
    m_Mesh = std::move(mesh);
}

/*============================================================================================================
    マテリアルを設定する
    引数:
      mat - 設定するマテリアルのshared_ptr
=============================================================================================================*/
void MeshRenderer::SetMaterial(std::shared_ptr<Material> mat)
{
    m_Material = std::move(mat);
}

/*============================================================================================================
    メッシュを取得する
    戻り値: メッシュのshared_ptr
=============================================================================================================*/
std::shared_ptr<Mesh> MeshRenderer::GetMesh() const
{
    return m_Mesh;
}

/*============================================================================================================
    マテリアルを取得する
    戻り値: マテリアルのshared_ptr
=============================================================================================================*/
std::shared_ptr<Material> MeshRenderer::GetMaterial() const
{
    return m_Material;
}
