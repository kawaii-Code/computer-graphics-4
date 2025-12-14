#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in mat4 instanceMatrix;

uniform mat4 view;
uniform mat4 proj;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main() {
    mat4 world = transpose(instanceMatrix);

    vec4 worldPos = world * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;

    mat3 normalMatrix = mat3(transpose(inverse(world)));
    Normal = normalize(normalMatrix * aNormal);

    TexCoord = aTexCoord;

    gl_Position = proj * view * worldPos;
}
