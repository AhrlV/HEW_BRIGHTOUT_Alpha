/*============================================================================================================

    レンダーシステムクラス実装 [render_system.cpp]
    レンダリング処理を統括するシングルトンクラスの実装。
    使用中のShaderを管理し、PerFrame定数バッファの効率的な更新を行う。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#include "rendering/render_system.h"
#include "resourcemanagement/constant_buffer.h"
#include "lifecycle/scene.h"
#include "rendering/camera.h"
#include "rendering/mesh_renderer.h"
#include "resourcemanagement/shader.h"
#include <stdexcept>
#include <vector>
#include <cstring>

/*============================================================================================================
    シングルトンインスタンスを取得する
    戻り値: RenderSystemの唯一のインスタンスへの参照
=============================================================================================================*/
RenderSystem& RenderSystem::Instance()
{
    static RenderSystem instance;
    return instance;
}

/*============================================================================================================
    デフォルトコンストラクタ
    ライティングパラメータを初期値で設定する。
=============================================================================================================*/
RenderSystem::RenderSystem()
    : m_AmbientColor(0.3f, 0.3f, 0.3f, 1.0f)
    , m_DirectionalColor(0.8f, 0.8f, 0.8f, 1.0f)
    , m_DirectionalDirection(0.0f, -1.0f, 0.0f, 0.0f)
{
}

/*============================================================================================================
    デストラクタ
=============================================================================================================*/
RenderSystem::~RenderSystem()
{
}

/*============================================================================================================
    Shaderを登録する
    ShaderFactoryから呼び出され、使用可能なShaderをRenderSystemに登録する。
    登録されたShaderはPerFrame定数バッファの更新対象となる。
    
    引数:
      shader - 登録するShaderのポインタ
=============================================================================================================*/
void RenderSystem::RegisterShader(Shader* shader)
{
    if (shader)
    {
        m_RegisteredShaders.insert(shader);
    }
}

/*============================================================================================================
    Shaderの登録を解除する
    引数:
      shader - 登録解除するShaderのポインタ
=============================================================================================================*/
void RenderSystem::UnregisterShader(Shader* shader)
{
    if (shader)
    {
        m_RegisteredShaders.erase(shader);
    }
}

/*============================================================================================================
    全てのPixelShaderのPerFrame定数バッファを更新する
    登録されている全てのPixelShaderに対して、PerFrame定数バッファを一度だけ更新する。
    リフレクション情報からオフセットを取得し、配列にデータを詰めて更新する。
    
    例外: 更新に失敗した場合はruntime_errorをスロー
=============================================================================================================*/
void RenderSystem::UpdateAllPixelShaderPerFrame()
{
    // 登録されている全てのShaderに対してPerFrame定数バッファを更新
    for (Shader* shader : m_RegisteredShaders)
    {
        if (!shader)
        {
            continue;
        }
        
        ShaderType type = shader->GetShaderType();
        
        // PixelShaderの場合
        if (type == ShaderType::Pixel)
        {
            PixelShader* pixelShader = static_cast<PixelShader*>(shader);
            
            // "PerFrame"定数バッファを取得
            ConstantBuffer* cb = pixelShader->GetConstantBufferByName("PerFrame");
            if (!cb)
            {
                continue;
            }
            
            // 定数バッファのサイズ分の配列を確保
            UINT bufferSize = cb->GetSize();
            std::vector<uint8_t> bufferData(bufferSize, 0);
            
            // 変数名とオフセットを取得してデータを詰める
            if (cb->HasVariable("ambient_color"))
            {
                UINT offset = cb->GetVariableOffset("ambient_color");
                memcpy(bufferData.data() + offset, &m_AmbientColor, sizeof(DirectX::XMFLOAT4));
            }
            
            if (cb->HasVariable("directional_color"))
            {
                UINT offset = cb->GetVariableOffset("directional_color");
                memcpy(bufferData.data() + offset, &m_DirectionalColor, sizeof(DirectX::XMFLOAT4));
            }
            
            if (cb->HasVariable("directional_direction"))
            {
                UINT offset = cb->GetVariableOffset("directional_direction");
                memcpy(bufferData.data() + offset, &m_DirectionalDirection, sizeof(DirectX::XMFLOAT4));
            }
            
            // 定数バッファを更新
            cb->Update(bufferData.data());
        }
    }
}

/*============================================================================================================
    全てのVertexShaderのPerFrame定数バッファを更新する
    登録されている全てのVertexShaderに対して、PerFrame定数バッファを更新する。
    カメラのビュー行列と射影行列をリフレクション情報から取得したオフセットを使用して設定する。
    
    引数:
      camera - カメラの参照
    例外: 更新に失敗した場合はruntime_errorをスロー
=============================================================================================================*/
void RenderSystem::UpdateAllVertexShaderPerFrame(const Camera& camera)
{
    // ビュー行列と射影行列を取得
    DirectX::XMMATRIX viewMatrix = camera.GetViewMatrix();
    DirectX::XMMATRIX projMatrix = camera.GetProjMatrix();
    
    // 転置してシェーダーに渡す形式に変換
    DirectX::XMMATRIX viewTranspose = DirectX::XMMatrixTranspose(viewMatrix);
    DirectX::XMMATRIX projTranspose = DirectX::XMMatrixTranspose(projMatrix);
    
    // 登録されている全てのShaderに対してPerFrame定数バッファを更新
    for (Shader* shader : m_RegisteredShaders)
    {
        if (!shader)
        {
            continue;
        }
        
        ShaderType type = shader->GetShaderType();
        
        // VertexShaderの場合
        if (type == ShaderType::Vertex)
        {
            VertexShader* vertexShader = static_cast<VertexShader*>(shader);
            
            // "PerFrame"定数バッファを取得
            ConstantBuffer* cb = vertexShader->GetConstantBufferByName("PerFrame");
            if (!cb)
            {
                continue;
            }
            
            // 定数バッファのサイズ分の配列を確保
            UINT bufferSize = cb->GetSize();
            std::vector<uint8_t> bufferData(bufferSize, 0);
            
            // 変数名とオフセットを取得してデータを詰める
            if (cb->HasVariable("view"))
            {
                UINT offset = cb->GetVariableOffset("view");
                memcpy(bufferData.data() + offset, &viewTranspose, sizeof(DirectX::XMMATRIX));
            }
            
            if (cb->HasVariable("proj"))
            {
                UINT offset = cb->GetVariableOffset("proj");
                memcpy(bufferData.data() + offset, &projTranspose, sizeof(DirectX::XMMATRIX));
            }
            
            // 定数バッファを更新
            cb->Update(bufferData.data());
        }
    }
}

/*============================================================================================================
    シーン全体のレンダリングを実行する
    Sceneから全てのCameraとMeshRendererを取得し、
    Cameraごとにビュー行列と射影行列を更新して、
    全てのMeshRendererを描画する。
    PerFrame定数バッファは全Shaderに対して一度だけ更新される。
    
    引数:
      scene - レンダリング対象のシーン
    例外: レンダリングに失敗した場合はruntime_errorをスロー
=============================================================================================================*/
void RenderSystem::Render(Scene* scene)
{
    if (!scene)
    {
        throw std::runtime_error("RenderSystem::Render - sceneがnullptrです");
    }
    
    // SceneからCameraとMeshRendererを取得
    auto cameras = scene->GetComponentsByType<Camera>();
    auto renderers = scene->GetComponentsByType<MeshRenderer>();
    
    // Cameraが存在しない場合は何もしない
    if (cameras.empty())
    {
        return;
    }
    
    // MeshRendererが存在しない場合は何もしない
    if (renderers.empty())
    {
        return;
    }
    
    // 全てのPixelShaderのPerFrame定数バッファを一度だけ更新
    UpdateAllPixelShaderPerFrame();
    
    // 各カメラで全てのMeshRendererを描画
    for (auto* camera : cameras)
    {
        if (camera)
        {
            // このカメラ用にVertexShaderのPerFrame定数バッファを更新
            UpdateAllVertexShaderPerFrame(*camera);
            
            // 各MeshRendererを描画
            // PerFrame定数バッファは既にUpdateAllPixelShaderPerFrame()で更新済みなので、
            // nullptrを渡してMeshRenderer側での更新をスキップさせる
            for (auto* renderer : renderers)
            {
                if (renderer)
                {
                    renderer->Render();
                }
            }
        }
    }
}

/*============================================================================================================
    登録されているShaderをクリアする
    シーン切り替え時などに使用する。
    登録されている全てのShaderの登録を解除する。
=============================================================================================================*/
void RenderSystem::Cleanup()
{
    m_RegisteredShaders.clear();
}

/*============================================================================================================
    環境光の色を設定する
    引数:
      color - 環境光の色（RGBA）
=============================================================================================================*/
void RenderSystem::SetAmbientColor(const DirectX::XMFLOAT4& color)
{
    m_AmbientColor = color;
}

/*============================================================================================================
    ディレクショナルライトの色を設定する
    引数:
      color - ディレクショナルライトの色（RGBA）
=============================================================================================================*/
void RenderSystem::SetDirectionalColor(const DirectX::XMFLOAT4& color)
{
    m_DirectionalColor = color;
}

/*============================================================================================================
    ディレクショナルライトの方向を設定する
    引数:
      direction - ライトの方向ベクトル（ワールド空間）
=============================================================================================================*/
void RenderSystem::SetDirectionalDirection(const DirectX::XMFLOAT4& direction)
{
    m_DirectionalDirection = direction;
}

/*============================================================================================================
    環境光の色を取得する
    戻り値: 環境光の色
=============================================================================================================*/
DirectX::XMFLOAT4 RenderSystem::GetAmbientColor() const
{
    return m_AmbientColor;
}

/*============================================================================================================
    ディレクショナルライトの色を取得する
    戻り値: ディレクショナルライトの色
=============================================================================================================*/
DirectX::XMFLOAT4 RenderSystem::GetDirectionalColor() const
{
    return m_DirectionalColor;
}

/*============================================================================================================
    ディレクショナルライトの方向を取得する
    戻り値: ライトの方向ベクトル
=============================================================================================================*/
DirectX::XMFLOAT4 RenderSystem::GetDirectionalDirection() const
{
    return m_DirectionalDirection;
}
