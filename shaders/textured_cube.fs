#version 330

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

out vec4 color;

void main() {
    Color=texture(ourTexture,TexCoord)* vec4(ourColor,1.0f);
}