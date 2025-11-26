/*============================================================================================================

    マテリアルクラス実装 [material.cpp]
    3Dモデルの材質情報の管理を行う。

    Author : Ryosuke Kageyama
    Date   : 2025/11/25

=============================================================================================================*/

#include "direct3D/material.h"
#include "direct3D/shader.h"
#include "direct3D/direct3d_device.h"
#include "direct3D/resource_factory.h"

/*============================================================================================================
    デフォルトコンストラクタ
    メンバ変数を初期化する。
=============================================================================================================*/
Material::Material()
    : Resource(ResourceClassID::Material)
    , Color(1.0f, 1.0f, 1.0f, 1.0f)
    , Texture(nullptr)
    , m_PixelShader(nullptr)
    , m_Sampler(nullptr)
    , m_BlendState(nullptr)
    , m_DepthState(nullptr)
{
    Initialize();
}

/*============================================================================================================
    識別名を指定するコンストラクタ
    引数:
      name - このマテリアルの識別名
=============================================================================================================*/
Material::Material(const std::wstring& name)
    : Resource(name, ResourceClassID::Material)
    , Color(1.0f, 1.0f, 1.0f, 1.0f)
    , Texture(nullptr)
    , m_PixelShader(nullptr)
    , m_Sampler(nullptr)
    , m_BlendState(nullptr)
    , m_DepthState(nullptr)
{
    Initialize();
}

/*============================================================================================================
    デストラクタ
    リソースを解放する。
=============================================================================================================*/
Material::~Material()
{
    Release();
}

/*============================================================================================================
    マテリアルの初期化
    デフォルトのブレンドステート、深度ステート、サンプラーステートを作成する。
    
    戻り値: 初期化に成功した場合true
=============================================================================================================*/
bool Material::Initialize()
{
    // GraphicsDeviceから取得
    static auto& device = GraphicsDevice::Instance();
	static auto d3dDevice = device.GetDevice();

    if (!d3dDevice)
    {
        return false;
    }
    
    CreateBlendState();
    CreateDepthState();
    CreateDefaultSampler();

    Texture = TextureFactory::Create(L"white.png");
    m_PixelShader = ShaderFactory::CreatePixelShader(L"PS_3D_Default.cso");
    
    return true;
}

/*============================================================================================================
    リソースを解放する
    すべてのレンダリングステートとテクスチャリソースを解放する。
=============================================================================================================*/
void Material::Release()
{
    m_BlendState.Reset();
    m_DepthState.Reset();
    m_Sampler.Reset();
    Texture->Release();
    
    m_PixelShader.reset();
}

/*============================================================================================================
    リソースが有効かどうかを判定する
    戻り値: ブレンドステートと深度ステートが有効な場合true
=============================================================================================================*/
bool Material::IsValid() const
{
    return m_BlendState.Get() != nullptr && m_DepthState.Get() != nullptr;
}

/*============================================================================================================
    ピクセルシェーダーを設定する
    引数:
      ps - 設定するピクセルシェーダーのshared_ptr
=============================================================================================================*/
void Material::SetPixelShader(std::shared_ptr<PixelShader> ps)
{
    m_PixelShader = ps;
}

/*============================================================================================================
    ピクセルシェーダーを取得する
    戻り値: ピクセルシェーダーのshared_ptr
=============================================================================================================*/
std::shared_ptr<PixelShader> Material::GetPixelShader() const
{
    return m_PixelShader;
}

/*============================================================================================================
    ブレンドステートを取得する
    戻り値: ブレンドステートのポインタ
=============================================================================================================*/
ID3D11BlendState* Material::GetBlendState() const
{
    return m_BlendState.Get();
}

/*============================================================================================================
    深度ステンシルステートを取得する
    戻り値: 深度ステンシルステートのポインタ
=============================================================================================================*/
ID3D11DepthStencilState* Material::GetDepthState() const
{
    return m_DepthState.Get();
}

/*============================================================================================================
    サンプラーステートを取得する
    戻り値: サンプラーステートのポインタ
=============================================================================================================*/
ID3D11SamplerState* Material::GetSampler() const
{
    return m_Sampler.Get();
}

/*============================================================================================================
    レンダリングステート再作成
    ブレンドステートと深度ステンシルステートを再作成する。
=============================================================================================================*/
void Material::RecreateStates()
{
    CreateBlendState();
    CreateDepthState();
}

/*============================================================================================================
    ブレンドステートを作成する
    アルファブレンドを無効にしたブレンドステートを作成する。
=============================================================================================================*/
void Material::CreateBlendState()
{
    // GraphicsDeviceから取得
    static auto& device = GraphicsDevice::Instance();
    static auto d3dDevice = device.GetDevice();

    if (!d3dDevice)
    {
        return;
    }

    m_BlendState.Reset();

    D3D11_BLEND_DESC bd{};
    bd.RenderTarget[0].BlendEnable = FALSE;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    d3dDevice->CreateBlendState(&bd, m_BlendState.GetAddressOf());
}

/*============================================================================================================
    深度ステンシルステートを作成する
    深度テストを有効にした深度ステンシルステートを作成する。
=============================================================================================================*/
void Material::CreateDepthState()
{
    // GraphicsDeviceから取得
    static auto& device = GraphicsDevice::Instance();
    static auto d3dDevice = device.GetDevice();

    if (!d3dDevice)
    {
        return;
    }

    m_DepthState.Reset();

    D3D11_DEPTH_STENCIL_DESC ds{};
    ds.DepthEnable = TRUE;
    ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    ds.DepthFunc = D3D11_COMPARISON_LESS;

    d3dDevice->CreateDepthStencilState(&ds, m_DepthState.GetAddressOf());
}

/*============================================================================================================
    デフォルトのサンプラーステートを作成する
    線形補間とラップモードを使用するサンプラーステートを作成する。
=============================================================================================================*/
void Material::CreateDefaultSampler()
{
    // GraphicsDeviceから取得
    static auto& device = GraphicsDevice::Instance();
    static auto d3dDevice = device.GetDevice();

    if (!d3dDevice)
    {
        return;
    }

    m_Sampler.Reset();

    D3D11_SAMPLER_DESC sd{};
    sd.Filter = D3D11_FILTER_ANISOTROPIC;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sd.MinLOD = 0;
    sd.MaxLOD = D3D11_FLOAT32_MAX;

    d3dDevice->CreateSamplerState(&sd, m_Sampler.GetAddressOf());
}
