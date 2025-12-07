#version 330

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

out vec2 texcoord;

void main() {
    gl_Position = proj * view * world * vec4(aPos, 1.0);
    texcoord = aTexCoord;
}

