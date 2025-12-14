#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform vec3 viewPos;

uniform vec3 ambientColor;

uniform bool pointLightEnabled;
uniform vec3 pointLightPos;
uniform vec3 pointLightColor;
uniform float pointLightIntensity;
uniform float pointLightConstant;
uniform float pointLightLinear;
uniform float pointLightQuadratic;

uniform bool dirLightEnabled;
uniform vec3 dirLightDirection;
uniform vec3 dirLightColor;
uniform float dirLightIntensity;

uniform bool spotLightEnabled;
uniform vec3 spotLightPos;
uniform vec3 spotLightDirection;
uniform vec3 spotLightColor;
uniform float spotLightIntensity;
uniform float spotLightCutOff;
uniform float spotLightOuterCutOff;
uniform float spotLightConstant;
uniform float spotLightLinear;
uniform float spotLightQuadratic;

out vec4 FragColor;

vec3 calcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(pointLightPos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    float distance = length(pointLightPos - fragPos);
    float attenuation = 1.0 / (pointLightConstant + pointLightLinear * distance +
                               pointLightQuadratic * distance * distance);
    vec3 diffuse = diff * pointLightColor * pointLightIntensity;
    vec3 specular = spec * pointLightColor * pointLightIntensity * 0.5;
    return (diffuse + specular) * attenuation;
}

vec3 calcDirLight(vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-dirLightDirection);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 diffuse = diff * dirLightColor * dirLightIntensity;
    vec3 specular = spec * dirLightColor * dirLightIntensity * 0.5;
    return diffuse + specular;
}

vec3 calcSpotLight(vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(spotLightPos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    float distance = length(spotLightPos - fragPos);
    float attenuation = 1.0 / (spotLightConstant + spotLightLinear * distance +
                               spotLightQuadratic * distance * distance);
    float theta = dot(lightDir, normalize(-spotLightDirection));
    float epsilon = spotLightCutOff - spotLightOuterCutOff;
    float intensity = clamp((theta - spotLightOuterCutOff) / epsilon, 0.0, 1.0);
    vec3 diffuse = diff * spotLightColor * spotLightIntensity;
    vec3 specular = spec * spotLightColor * spotLightIntensity * 0.5;
    return (diffuse + specular) * attenuation * intensity;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec4 texColor = texture(ourTexture, TexCoord);
    vec3 objectColor = texColor.rgb;
    vec3 result = ambientColor * objectColor;

    if (pointLightEnabled) {
        result += calcPointLight(norm, FragPos, viewDir) * objectColor;
    }
    if (dirLightEnabled) {
        result += calcDirLight(norm, viewDir) * objectColor;
    }
    if (spotLightEnabled) {
        result += calcSpotLight(norm, FragPos, viewDir) * objectColor;
    }

    FragColor = vec4(result, 1.0);
}
