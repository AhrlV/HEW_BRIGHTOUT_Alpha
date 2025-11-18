cbuffer VS_CONSTANT_BUFFER0 : register(b0)
{
    float4x4 proj;
}

cbuffer VS_CONSTANT_BUFFER1 : register(b1)
{
    float4x4 world;
}


struct VS_INPUT
{
    float4 posL  : POSITION0;
    float4 color : COLOR0;
    float2 tex   : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    float2 tex : TEXCOORD0;
};

VS_OUTPUT main( VS_INPUT vsin )
{
    VS_OUTPUT vsout;
    
    float4x4 mtx = mul(world, proj);
    vsout.posH = mul(vsin.posL, mtx);
    
    vsout.color = vsin.color;
    vsout.tex = vsin.tex;
    
    return vsout;
}