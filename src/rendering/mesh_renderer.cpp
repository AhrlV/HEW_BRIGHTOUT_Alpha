/*============================================================================================================

    メッシュレンダラークラス実装 [mesh_renderer.cpp]
    メッシュとマテリアルを使用して3Dモデルを描画する。

    Author : Ryosuke Kageyama
    Date   : 2025/11/25

=============================================================================================================*/

#include "rendering/mesh_renderer.h"
#include "direct3D/direct3d_device.h"
#include "resourcemanagement/constant_buffer.h"
#include <stdexcept>
#include <DirectXMath.h>
#include <vector>
#include "resourcemanagement/material_factory.h"
#include "physics/transform.h"
#include "lifecycle/gameobject.h"

using namespace DirectX;

/*============================================================================================================
    デフォルトコンストラクタ
    メンバ変数を初期化する。
=============================================================================================================*/
MeshRenderer::MeshRenderer()
    : m_Mesh(nullptr)
    , m_Material(nullptr)
    , material(m_PropertyBlock)
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
=============================================================================================================*/
void MeshRenderer::Render()
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
        SetupVertexShader(vertexShader.get());
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
    頂点シェーダーをパイプラインに設定し、定数バッファを更新する。
    - CameraがPerFrame定数バッファ(view, proj)を更新
    - MeshがPerObject定数バッファ(world)を更新
    
    引数:
      vertexShader - 頂点シェーダーのポインタ
=============================================================================================================*/
void MeshRenderer::SetupVertexShader(VertexShader* vertexShader)
{
    if (!vertexShader || !vertexShader->IsValid())
    {
        return;
    }

    // 頂点シェーダーと入力レイアウトをパイプラインに設定
    vertexShader->SetToContext();

    // MeshがPerObject定数バッファ(ワールド行列)を更新
    auto tf = m_Owner->GetComponent<TransForm>();
    XMMATRIX worldMatrix = tf->GetWorldMatrix();
    m_Mesh->UpdateVertexShaderPerObject(worldMatrix, vertexShader);

    // 定数バッファをシェーダーに設定
    vertexShader->SetConstantBuffers(0);
}

/*============================================================================================================
    ピクセルシェーダーを設定する
    ピクセルシェーダーをパイプラインに設定し、定数バッファを更新する。
    - MaterialがPerMaterial定数バッファ(base_color, diffuse_color)を更新
    - MaterialPropertyBlockで値をオーバーライド
    - PropertyBlockにシェーダーに存在しない変数がある場合はエラーを出力
    リフレクション情報からオフセットを取得し、配列にデータを詰めて更新する。
    
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


    // MaterialPropertyBlockの値を検証
    // シェーダーに存在しない変数がある場合はエラーを出力
    auto propertyNames = m_PropertyBlock.GetPropertyNames();
    for (const auto& propName : propertyNames)
    {
        // PerMaterial定数バッファに変数が存在するか確認
        if (!pixelShader->HasConstantBufferVariable("PerMaterial", propName))
        {
            throw std::runtime_error(
                "MeshRenderer::SetupPixelShader - PropertyBlockに指定された変数名 '" + propName + 
                "' はシェーダーのPerMaterial定数バッファに存在しません"
            );
        }
    }

    // "PerMaterial"定数バッファを取得
    ConstantBuffer* cb = pixelShader->GetConstantBufferByName("PerMaterial");
    if (!cb)
    {
        throw std::runtime_error("MeshRenderer::SetupPixelShader - PerMaterial定数バッファが見つかりません");
    }
    
    // 定数バッファのサイズ分の配列を確保
    UINT bufferSize = cb->GetSize();
    std::vector<uint8_t> bufferData(bufferSize, 0);
    
    // マテリアルのデフォルト値を取得してデータを詰める
    if (cb->HasVariable("base_color"))
    {
        DirectX::XMFLOAT4 value = m_Material->BaseColor;
        
        // PropertyBlockでオーバーライド
        DirectX::XMFLOAT4 overrideValue;
        if (m_PropertyBlock.GetFloat4("base_color", overrideValue))
        {
            value = overrideValue;
        }
        
        UINT offset = cb->GetVariableOffset("base_color");
        memcpy(bufferData.data() + offset, &value, sizeof(DirectX::XMFLOAT4));
    }
    
    if (cb->HasVariable("diffuse_color"))
    {
        DirectX::XMFLOAT4 value = m_Material->DiffuseColor;
        
        // PropertyBlockでオーバーライド
        DirectX::XMFLOAT4 overrideValue;
        if (m_PropertyBlock.GetFloat4("diffuse_color", overrideValue))
        {
            value = overrideValue;
        }
        
        UINT offset = cb->GetVariableOffset("diffuse_color");
        memcpy(bufferData.data() + offset, &value, sizeof(DirectX::XMFLOAT4));
    }
    
    // 定数バッファを更新
    cb->Update(bufferData.data());

    // 定数バッファをシェーダーに設定
    pixelShader->SetConstantBuffers(0);

    // テクスチャの設定
    // PropertyBlockにテクスチャが設定されている場合はそれを優先、なければマテリアルのテクスチャを使用
    std::shared_ptr<Texture> textureToUse = m_Material->Texture;
    
    if (m_PropertyBlock.HasTexture())
    {
        textureToUse = m_PropertyBlock.GetTexture();
    }
    
    if (textureToUse)
    {
        ID3D11ShaderResourceView* srv = textureToUse.get()->GetShaderResourceView();
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
std::shared_ptr<Material> MeshRenderer::GetSharedMaterial() const
{
    return m_Material;
}

/*============================================================================================================
    マテリアルプロパティブロックを取得する
    戻り値: MaterialPropertyBlockの参照
=============================================================================================================*/
MaterialPropertyBlock& MeshRenderer::GetMaterial()
{
    return m_PropertyBlock;
}

/*============================================================================================================
    マテリアルプロパティブロックを取得する（const版）
    戻り値: MaterialPropertyBlockのconst参照
=============================================================================================================*/
const MaterialPropertyBlock& MeshRenderer::GetPropertyBlock() const
{
    return m_PropertyBlock;
}
