/*============================================================================================================

    テクスチャークラス実装 [texture.cpp]
    テクスチャーリソースの読み込みと管理を行う。

    Author : Ryosuke Kageyama
    Date   : 2025/05/07

=============================================================================================================*/

#include "resourcemanagement/texture.h"
#include "DirectXTex.h"
#include <stdexcept>
#include "direct3D/direct3d_device.h"

using namespace DirectX;

/*============================================================================================================
    デフォルトコンストラクタ
    メンバ変数を初期化する。
=============================================================================================================*/
Texture::Texture()
    : Object()
    , m_ShaderResourceView(nullptr)
    , m_Width(0)
    , m_Height(0)
{
    m_ClassID = ResourceClassID::Texture;
}

/*============================================================================================================
    識別名を指定するコンストラクタ
    引数:
      name - このテクスチャの識別名
=============================================================================================================*/
Texture::Texture(const std::wstring& name)
    : Object()
    , m_ShaderResourceView(nullptr)
    , m_Width(0)
    , m_Height(0)
{
    m_Name = name;
    m_ClassID = ResourceClassID::Texture;
}

/*============================================================================================================
    デストラクタ
    リソースを解放する。
=============================================================================================================*/
Texture::~Texture()
{
    Release();
}

/*============================================================================================================
    テクスチャの初期化と読み込み
    指定したファイルからテクスチャを読み込み、シェーダーリソースビューを作成する。
    
    引数:
      filename - 読み込むテクスチャファイルのパス
      generateMipMaps - ミップマップを生成するかどうか
    戻り値: 初期化に成功した場合true
    例外: 読み込みに失敗した場合はruntime_error例外をスロー
=============================================================================================================*/
bool Texture::Initialize(const std::wstring& filename, bool generateMipMaps)
{
    // GraphicsDeviceから取得
    static auto& device = GraphicsDevice::Instance();
    static auto d3dDevice = device.GetDevice();

    // 引数チェック
    if (!d3dDevice)
    {
        throw std::runtime_error("Texture::Initialize - デバイスがnullptrです");
    }

    if (filename.empty())
    {
        throw std::runtime_error("Texture::Initialize - ファイル名が空です");
    }

    // テクスチャメタデータとイメージデータ
    TexMetadata metadata;
    ScratchImage image;

    // 画像ファイルの読み込み
    HRESULT hr = LoadFromWICFile(filename.c_str(), WIC_FLAGS_NONE, &metadata, image);
    if (FAILED(hr))
    {
        throw std::runtime_error("Texture::Initialize - 画像ファイルの読み込みに失敗しました");
    }

    // 画像サイズを保存
    m_Width = static_cast<unsigned int>(metadata.width);
    m_Height = static_cast<unsigned int>(metadata.height);

    // ミップマップの生成
    if (generateMipMaps)
    {
        ScratchImage mipChain;
        hr = GenerateMipMaps(
            image.GetImages(),
            image.GetImageCount(),
            image.GetMetadata(),
            TEX_FILTER_DEFAULT,
            0,
            mipChain
        );

        if (SUCCEEDED(hr))
        {
            image = std::move(mipChain);
            metadata = image.GetMetadata();
        }
    }

    // シェーダーリソースビューの生成
    hr = CreateShaderResourceView(
        d3dDevice,
        image.GetImages(),
        image.GetImageCount(),
        metadata,
        m_ShaderResourceView.GetAddressOf()
    );

    if (FAILED(hr))
    {
        throw std::runtime_error("Texture::Initialize - シェーダーリソースビューの作成に失敗しました");
    }

    return true;
}

/*============================================================================================================
    シェーダーへのテクスチャ設定
    このテクスチャをピクセルシェーダーの指定したスロットに設定する。
    
    引数:
      slot - シェーダーリソーススロット番号
=============================================================================================================*/
void Texture::SetToShader(UINT slot) const
{
    // GraphicsDeviceから取得
    static auto& device = GraphicsDevice::Instance();
    static auto context = device.GetContext();

    if (!context)
    {
        throw std::runtime_error("Texture::SetToShader - コンテキストがnullptrです");
    }

    if (!IsValid())
    {
        throw std::runtime_error("Texture::SetToShader - テクスチャが無効です");
    }

    ID3D11ShaderResourceView* srv = m_ShaderResourceView.Get();
    context->PSSetShaderResources(slot, 1, &srv);
}

/*============================================================================================================
    リソースを解放する
    シェーダーリソースビューを解放し、サイズ情報をリセットする。
=============================================================================================================*/
void Texture::Release()
{
    m_ShaderResourceView.Reset();
    m_Width = 0;
    m_Height = 0;
}

/*============================================================================================================
    リソースが有効かどうかを判定する
    戻り値: シェーダーリソースビューが有効な場合true
=============================================================================================================*/
bool Texture::IsValid() const
{
    return m_ShaderResourceView.Get() != nullptr;
}

/*============================================================================================================
    シェーダーリソースビューを取得する
    戻り値: シェーダーリソースビューのポインタ
=============================================================================================================*/
ID3D11ShaderResourceView* Texture::GetShaderResourceView() const
{
    return m_ShaderResourceView.Get();
}

/*============================================================================================================
    テクスチャの幅を取得する
    戻り値: テクスチャの幅（ピクセル）
=============================================================================================================*/
unsigned int Texture::GetWidth() const
{
    return m_Width;
}

/*============================================================================================================
    テクスチャの高さを取得する
    戻り値: テクスチャの高さ（ピクセル）
=============================================================================================================*/
unsigned int Texture::GetHeight() const
{
    return m_Height;
}

/*============================================================================================================
    テクスチャのサイズを取得する
    戻り値: テクスチャのサイズ（幅と高さ）
=============================================================================================================*/
DirectX::XMUINT2 Texture::GetSize() const
{
    return DirectX::XMUINT2(m_Width, m_Height);
}

/*============================================================================================================
    テクスチャデータを直接設定する（Factoryからの使用を想定）
    
    引数:
      srv - シェーダーリソースビュー
      width - テクスチャの幅
      height - テクスチャの高さ
=============================================================================================================*/
void Texture::SetTextureData(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv, unsigned int width, unsigned int height)
{
    m_ShaderResourceView = srv;
    m_Width = width;
    m_Height = height;
}
