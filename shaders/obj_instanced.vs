#version 330

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in mat4 instanceMatrix;

uniform mat4 view;
uniform mat4 proj;

out vec2 TexCoord;

void main() {
    mat4 world = transpose(instanceMatrix);
    gl_Position = proj * view * world * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
