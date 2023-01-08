struct VSInput
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

cbuffer TileConstantBuffer : register(b0)
{
    float4x4 position;
    float4 color;
};

PSInput VS(VSInput input)
{
    PSInput result;    
    
    result.position = mul(float4(input.position, 1.0f), position);
        
    result.texCoord = input.texCoord;

    return result;
}

Texture2D tile : register(t0);
SamplerState gSamPoint : register(s0);

float4 PS(PSInput input) : SV_TARGET
{
    float4 texColor = tile.Sample(gSamPoint, input.texCoord) * color;
    
    return texColor;
}




