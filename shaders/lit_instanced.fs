#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D ourTexture;
uniform vec3 viewPos;
uniform vec3 ambientColor;
uniform int lightingModel;

uniform bool  pointLightEnabled;
uniform vec3  pointLightPos;
uniform vec3  pointLightColor;
uniform float pointLightIntensity;
uniform float pointLightConstant;
uniform float pointLightLinear;
uniform float pointLightQuadratic;

uniform bool  dirLightEnabled;
uniform vec3  dirLightDirection;
uniform vec3  dirLightColor;
uniform float dirLightIntensity;

uniform bool  spotLightEnabled;
uniform vec3  spotLightPos;
uniform vec3  spotLightDirection;
uniform vec3  spotLightColor;
uniform float spotLightIntensity;
uniform float spotLightCutOff;
uniform float spotLightOuterCutOff;
uniform float spotLightConstant;
uniform float spotLightLinear;
uniform float spotLightQuadratic;

const float PI = 3.14159265359;

vec3 phongLighting(vec3 N, vec3 V, vec3 L, vec3 lightColor)
{
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(V, R), 0.0), 32.0);

    vec3 diffuse  = diff * lightColor;
    vec3 specular = spec * lightColor;

    return diffuse + specular;
}

vec3 toonLighting(vec3 N, vec3 L, vec3 lightColor)
{
    float intensity = dot(N, L);

    float level;
    if (intensity > 0.95) level = 1.0;
    else if (intensity > 0.5) level = 0.7;
    else if (intensity > 0.25) level = 0.4;
    else level = 0.1;

    return lightColor * level;
}


vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 ashikhminDiffuse(vec3 albedo, float NdotL, float NdotV)
{
    return albedo * (28.0 / (23.0 * PI)) *
           (1.0 - pow(1.0 - NdotL * 0.5, 5.0)) *
           (1.0 - pow(1.0 - NdotV * 0.5, 5.0));
}

float ashikhminSpecular(vec3 N, vec3 H, float roughness)
{
    float NdotH = max(dot(N, H), 0.0);
    return (roughness + 2.0) / (2.0 * PI) * pow(NdotH, roughness);
}

vec3 ashikhminLighting(
    vec3 N, vec3 V, vec3 L,
    vec3 albedo,
    vec3 lightColor
) {
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    vec3 F0 = vec3(0.6);
    vec3 F = fresnelSchlick(VdotH, F0);

    vec3 diffuse = ashikhminDiffuse(albedo, NdotL, NdotV);
    float specFactor = ashikhminSpecular(N, H, 40.0);
    vec3 specular = F * specFactor;

    return (diffuse + specular) * lightColor * NdotL;
}

void main()
{
    vec3 albedo = texture(ourTexture, TexCoord).rgb;

    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    vec3 result = ambientColor * albedo;

    if (pointLightEnabled)
    {
        vec3 L = normalize(pointLightPos - FragPos);
        float distance = length(pointLightPos - FragPos);
        float attenuation =
            1.0 / (pointLightConstant +
                   pointLightLinear * distance +
                   pointLightQuadratic * distance * distance);

        vec3 lightResult;

        if (lightingModel == 0)
            lightResult = phongLighting(N, V, L, pointLightColor);
        else if (lightingModel == 1)
            lightResult = toonLighting(N, L, pointLightColor);
        else
            lightResult = ashikhminLighting(N, V, L, albedo, pointLightColor);

        result += lightResult * attenuation * pointLightIntensity;
    }

    if (dirLightEnabled)
    {
        vec3 L = normalize(-dirLightDirection);
        vec3 lightResult;

        if (lightingModel == 0)
            lightResult = phongLighting(N, V, L, dirLightColor);
        else if (lightingModel == 1)
            lightResult = toonLighting(N, L, dirLightColor);
        else
            lightResult = ashikhminLighting(N, V, L, albedo, dirLightColor);

        result += lightResult * dirLightIntensity;
    }

    if (spotLightEnabled)
    {
        vec3 L = normalize(spotLightPos - FragPos);
        float theta = dot(L, normalize(-spotLightDirection));

        float epsilon = spotLightCutOff - spotLightOuterCutOff;
        float intensity = clamp((theta - spotLightOuterCutOff) / epsilon, 0.0, 1.0);

        float distance = length(spotLightPos - FragPos);
        float attenuation =
            1.0 / (spotLightConstant +
                   spotLightLinear * distance +
                   spotLightQuadratic * distance * distance);

        vec3 lightResult;

        if (lightingModel == 0)
            lightResult = phongLighting(N, V, L, spotLightColor);
        else if (lightingModel == 1)
            lightResult = toonLighting(N, L, spotLightColor);
        else
            lightResult = ashikhminLighting(N, V, L, albedo, spotLightColor);

        result += lightResult * attenuation * intensity * spotLightIntensity;
    }

    FragColor = vec4(result, 1.0);
}
