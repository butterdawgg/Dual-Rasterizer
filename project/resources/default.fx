#define MAX_LIGHTS 8
#define PI 3.14159265359f

#define SAMPLING_MODE_POINT 0
#define SAMPLING_MODE_LINEAR 1
#define SAMPLING_MODE_ANISOTROPIC 2



struct Light
{
    float3 direction;
    float padding1; // padding for 16-byte alignment
    float3 color;
    float intensity;
};

// Matrices
float4x4 gWorld;
float4x4 gViewProj;

// Camera world position
float3 gCameraPosition;

// Light array
int gNumLights;
Light gLights[MAX_LIGHTS];

// Material properties
float3 gDiffuseColor;
float3 gSpecularColor;
float3 gAmbientColor;
float gDiffuseReflectance;
float gSpecularReflectance;
float gGlossiness;

// Textures
Texture2D gDiffuseMap;
Texture2D gNormalMap;
Texture2D gSpecularMap;
Texture2D gGlossinessMap;

// Rendering settings
int gSamplingMode;
bool gUseNormalMap;

// Sampler states
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
    AddressU = Wrap;
    AddressV = Wrap;
};

// Rasterizer states
RasterizerState gRasterizerState
{
    CullMode = back;
    FrontCounterClockwise = false;
};

RasterizerState gRasterizerNoCullState
{
    CullMode = none;
    FrontCounterClockwise = false;
};

// Blend states
BlendState gBlendState
{
    BlendEnable[0] = false;
};

BlendState gAlphaBlendState
{
    BlendEnable[0] = true;
    SrcBlend = src_alpha;
    DestBlend = inv_src_alpha;
    BlendOp = add;
    SrcBlendAlpha = one;
    DestBlendAlpha = zero;
    BlendOpAlpha = add;
    RenderTargetWriteMask[0] = 0x0F;
};

// Depth stencis states
DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = all;
    DepthFunc = less;
    StencilEnable = false;
};

DepthStencilState gNoDepthWriteState
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less;
    StencilEnable = false;
};

// I/O structs
struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Color : COLOR;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : WORLDPOS;
    float3 Color : COLOR;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};



// ---------------------
// --- Vertex shader ---
// ---------------------

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;

    // Transform position to world space
    output.WorldPosition = mul(float4(input.Position, 1.0f), gWorld);

    // Transform to clip space
    output.Position = mul(output.WorldPosition, gViewProj);

    // Transform normal and tangent to world space
    output.Normal = normalize(mul(input.Normal, (float3x3) gWorld));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) gWorld));

    // Pass through texture coordinates and color
    output.TexCoord = input.TexCoord;
    output.Color = input.Color;

    return output;
}



// ------------------------
// --- Helper functions ---
// ------------------------

float4 SampleTexture(Texture2D tex, float2 uv)
{
    if (gSamplingMode == SAMPLING_MODE_POINT)
        return tex.Sample(samPoint, uv);
    else if (gSamplingMode == SAMPLING_MODE_LINEAR)
        return tex.Sample(samLinear, uv);
    else if (gSamplingMode == SAMPLING_MODE_ANISOTROPIC)
        return tex.Sample(samAnisotropic, uv);
    else
        return tex.Sample(samPoint, uv);
}

float3 CalculateNormal(float3 normal, float3 tangent, float2 texCoord)
{
    if (!gUseNormalMap)
        return normalize(normal);

    // Sample normal map
    float3 sampledNormal = SampleTexture(gNormalMap, texCoord).rgb;

    // Remap from [0,1] to [-1,1]
    sampledNormal = 2.0f * sampledNormal - 1.0f;

    // Build TBN matrix
    float3 bitangent = normalize(cross(normal, tangent));
    float3x3 TBN = float3x3(tangent, bitangent, normal);

    // Transform normal from tangent space to world space
    return normalize(mul(sampledNormal, TBN));
}

float3 Lambert(float3 kd, float nDotL)
{
    return kd * nDotL / PI;
}

float3 BlinnPhong(float3 ks, float phongExp, float3 l, float3 v, float3 n)
{
    float3 h = normalize(l + v);
    float nDotH = max(dot(n, h), 0.0f);
    return ks * pow(nDotH, phongExp);
}



// ---------------------
// --- Pixel shaders ---
// ---------------------

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    // Sample textures
    float3 diffuseColor = SampleTexture(gDiffuseMap, input.TexCoord).rgb;
    float3 specularColor = SampleTexture(gSpecularMap, input.TexCoord).rgb;
    float glossiness = SampleTexture(gGlossinessMap, input.TexCoord).r;

    // Compute normal
    float3 normal = CalculateNormal(input.Normal, input.Tangent, input.TexCoord);

    // Compute view direction
    float3 viewDirection = normalize(gCameraPosition - input.WorldPosition.xyz);

    // Compute material coefficients
    float3 kd = gDiffuseColor * diffuseColor;
    float3 ks = gSpecularColor * specularColor;
    float phongExponent = glossiness * gGlossiness;

    // Compute ambient term
    float3 ambient = gAmbientColor * diffuseColor;

    // Calculate lighting
    float3 accumulatedLight = float3(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < gNumLights; ++i)
    {
        float3 lightDir = normalize(-gLights[i].direction);
        float nDotL = max(dot(normal, lightDir), 0.0f);

        if (nDotL <= 0.0f)
            continue;

        float3 diffuse = Lambert(kd, nDotL) * gDiffuseReflectance;

        float3 specular = BlinnPhong(ks, phongExponent,
            lightDir, viewDirection, normal) * gSpecularReflectance;

        float3 lightContribution;

        accumulatedLight += (diffuse + specular) *
                            gLights[i].color *
                            gLights[i].intensity;
    }

    float3 finalColor = ambient + accumulatedLight;

    return float4(finalColor, 1.0f);
}


float4 PS_Transparent(VS_OUTPUT input) : SV_TARGET
{
    float4 diffuseColor = SampleTexture(gDiffuseMap, input.TexCoord);

    return diffuseColor + float4(gAmbientColor, 0.0f);
}



// ------------------
// --- Techniques ---
// ------------------

technique11 OpaqueTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}

technique11 TransparentTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerNoCullState);
        SetDepthStencilState(gNoDepthWriteState, 0);
        SetBlendState(gAlphaBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Transparent()));
    }
}