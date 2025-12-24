#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 lightDir;   
uniform vec3 lightColor;
uniform vec3 viewPos;

uniform sampler2D diffuseTex;

void main()
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(-lightDir);

    float diff = max(dot(N, L), 0.0);

    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 32.0);

    vec3 albedo = texture(diffuseTex, TexCoord).rgb;

    vec3 color = albedo * (0.2 + diff) * lightColor + spec * lightColor * 0.2;

    FragColor = vec4(color, 1.0);
}
