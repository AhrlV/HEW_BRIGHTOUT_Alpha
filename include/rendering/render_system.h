/*============================================================================================================

    レンダーシステムクラス [render_system.h]
    レンダリング処理を統括するシングルトンクラス。
    Sceneから全てのCameraとMeshRendererを取得し、適切な順序でレンダリングを実行する。
    使用中のShaderを管理し、PerFrame定数バッファの効率的な更新を行う。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <DirectXMath.h>
#include <set>
#include <vector>

// 前方宣言
class Scene;
class Camera;
class MeshRenderer;
class VertexShader;
class PixelShader;
class Shader;

/*============================================================================================================
    レンダーシステムクラス
    レンダリング処理を統括するシングルトンクラス。
    PerFrame定数バッファの管理とレンダリングの実行を担当する。
    使用中のShaderを追跡し、PerFrame定数バッファの重複更新を防ぐ。
=============================================================================================================*/
class RenderSystem
{
public:
    /*========================================================================================================
        シングルトンアクセス
    ========================================================================================================*/
    
    // シングルトンインスタンスを取得する
    // 戻り値: RenderSystemの唯一のインスタンスへの参照
    static RenderSystem& Instance();

    /*========================================================================================================
        Shader登録管理
    ========================================================================================================*/
    
    // Shaderを登録する
    // ShaderFactoryから呼び出され、使用可能なShaderをRenderSystemに登録する
    // 引数:
    //   shader - 登録するShaderのポインタ
    void RegisterShader(Shader* shader);
    
    // Shaderの登録を解除する
    // 引数:
    //   shader - 登録解除するShaderのポインタ
    void UnregisterShader(Shader* shader);

    /*========================================================================================================
        レンダリング処理
    ========================================================================================================*/
    
    // シーン全体のレンダリングを実行する
    // Sceneから全てのCameraとMeshRendererを取得し、
    // Cameraごとにビュー行列と射影行列を更新して、
    // 全てのMeshRendererを描画する。
    // 引数:
    //   scene - レンダリング対象のシーン
    // 例外: レンダリングに失敗した場合はruntime_errorをスロー
    void Render(Scene* scene);
    
    // 登録されているShaderをクリアする
    // シーン切り替え時などに使用する
    void Cleanup();

    /*========================================================================================================
        ライティング設定
    ========================================================================================================*/
    
    // 環境光の色を設定する
    // 引数:
    //   color - 環境光の色（RGBA）
    void SetAmbientColor(const DirectX::XMFLOAT4& color);
    
    // ディレクショナルライトの色を設定する
    // 引数:
    //   color - ディレクショナルライトの色（RGBA）
    void SetDirectionalColor(const DirectX::XMFLOAT4& color);
    
    // ディレクショナルライトの方向を設定する
    // 引数:
    //   direction - ライトの方向ベクトル（ワールド空間）
    void SetDirectionalDirection(const DirectX::XMFLOAT4& direction);
    
    // 環境光の色を取得する
    // 戻り値: 環境光の色
    DirectX::XMFLOAT4 GetAmbientColor() const;
    
    // ディレクショナルライトの色を取得する
    // 戻り値: ディレクショナルライトの色
    DirectX::XMFLOAT4 GetDirectionalColor() const;
    
    // ディレクショナルライトの方向を取得する
    // 戻り値: ライトの方向ベクトル
    DirectX::XMFLOAT4 GetDirectionalDirection() const;

private:
    /*========================================================================================================
        コンストラクタ・デストラクタ（シングルトン）
    ========================================================================================================*/
    
    // デフォルトコンストラクタ（private）
    RenderSystem();
    
    // デストラクタ
    ~RenderSystem();
    
    // コピー禁止
    RenderSystem(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;
    
    // ムーブ禁止
    RenderSystem(RenderSystem&&) = delete;
    RenderSystem& operator=(RenderSystem&&) = delete;

    /*========================================================================================================
        内部ヘルパー関数
    ========================================================================================================*/
    
    // 全てのPixelShaderのPerFrame定数バッファを更新する
    // 各PixelShaderに対して一度だけ更新を行う
    // 例外: 更新に失敗した場合はruntime_errorをスロー
    void UpdateAllPixelShaderPerFrame();
    
    // 全てのVertexShaderのPerFrame定数バッファを更新する
    // 各VertexShaderに対してカメラのビュー行列・射影行列を更新
    // 引数:
    //   camera - カメラの参照
    // 例外: 更新に失敗した場合はruntime_errorをスロー
    void UpdateAllVertexShaderPerFrame(const Camera& camera);

    /*========================================================================================================
        メンバ変数
    ========================================================================================================*/
    
    // 環境光の色
    DirectX::XMFLOAT4 m_AmbientColor;
    
    // ディレクショナルライトの色
    DirectX::XMFLOAT4 m_DirectionalColor;
    
    // ディレクショナルライトの方向
    DirectX::XMFLOAT4 m_DirectionalDirection;
    
    // 登録されているShaderのセット（重複なし）
    std::set<Shader*> m_RegisteredShaders;
};

#endif // RENDER_SYSTEM_H
