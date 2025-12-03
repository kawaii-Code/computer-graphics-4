#version 330

layout(location = 0) in vec3 aPos;      
layout(location = 1) in vec3 aColor;    
layout(location = 2) in vec2 aTexCoord;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 world;

out vec3 color;
out vec2 texcoord;

void main() {
    vec4 pos = proj * view * world * vec4(aPos, 1.0);

    gl_Position = pos;
    color = aColor;
    texcoord = aTexCoord;
}
