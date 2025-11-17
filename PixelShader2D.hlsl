float4 color;

struct PS_INPUT
{
    float4 posH  : SV_POSITION;
    float4 color : COLOR0;
    float2 tex   : TEXCOORD0;
};


Texture2D major_texture; //　テクスチャ
SamplerState major_sampler; // テクスチャサンプラ

float4 main(PS_INPUT psin) : SV_TARGET
{
    float4 texcolor = major_texture.Sample(major_sampler, psin.tex);
    
    return texcolor * color;
}