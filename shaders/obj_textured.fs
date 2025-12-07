#version 330

in vec2 texcoord;

uniform sampler2D ourTexture;

out vec4 FragColor;

void main() {
    // Получаем цвет из текстуры
    vec4 texColor = texture(ourTexture, texcoord);

    // Используем цвет текстуры с полной непрозрачностью
    FragColor = vec4(texColor.rgb, 1.0);
}
