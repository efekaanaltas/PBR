#version 330 core

struct Material
{
    sampler2D diffuse;
    sampler2D roughness;
    sampler2D normal;
    sampler2D ao;
    sampler2D metallic;
    sampler2D height;
};

out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform vec3 viewPos;

uniform Material material;
uniform samplerCube depthCubemap;
uniform samplerCube skybox;

uniform float nearPlane;
uniform float farPlane;

uniform float heightScale;

const float PI = 3.14159265359;

vec3 GetNormalFromMap()
{
    vec3 tangentNormal = texture(material.normal, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdx(TexCoords);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float ShadowCalculation(vec3 fragPos)
{
    vec3 sampleOffsetDirections[20] = vec3[]
    (
       vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
       vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
       vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
       vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
       vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );   

    vec3 fragToLight = fragPos - lightPositions[0];
    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.15;
    float samples = 20.0;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / farPlane)) / 25.0;
    
    for(int i = 0; i < samples; i++)
    {
        float closestDepth = texture(depthCubemap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= farPlane;
        if(currentDepth - bias > closestDepth)
        {
            shadow += 1.0;
        }
    }

    return shadow /= float(samples);
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    const float minLayers = 8.0;
    const float maxLayers = 32.0;
    float layerCount = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));
    float layerDepth = 1.0 / layerCount;
    float currentLayerDepth = 0.0;

    vec2 P = viewDir.xy * heightScale;
    vec2 deltaTexCoords = P / layerCount;

    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(material.height, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue)
    {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(material.height, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(material.height, prevTexCoords).r - currentLayerDepth + layerDepth;

    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2-1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

void main()
{
    vec3 albedo = texture(material.diffuse, TexCoords).rgb;
    float metallic = 1.0;//texture(material.metallic, TexCoords).r;
    float roughness = 0.0;//texture(material.roughness, TexCoords).r;
    float ao = texture(material.ao, TexCoords).r;

    vec3 norm = texture(material.normal, TexCoords).xyz;

    vec3 N = GetNormalFromMap();
    vec3 V = normalize(viewPos - WorldPos);

    vec3 F0 = vec3(0.04);
    F0      = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; i++)
    {
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F  = FresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator     = NDF * G * F;
        float denominator  = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular      = numerator / denominator;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color   = ambient + Lo;

    //vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    //vec2 texCoords = ParallaxMapping(TexCoords, viewDir);
    //if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
    //{
    //    discard;
    //}
    
    vec3 tangentNormal = texture(material.normal, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdx(TexCoords);

    /*vec3 */N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    FragColor = vec4(N, 1.0);
} 