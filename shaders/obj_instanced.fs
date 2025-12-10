#version 330

in vec2 TexCoord;

uniform sampler2D ourTexture;

out vec4 FragColor;

void main() {
    vec4 texColor = texture(ourTexture, TexCoord);
    FragColor = vec4(texColor.rgb, 1.0);
}

