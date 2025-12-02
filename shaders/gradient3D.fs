#version 330

uniform float time;

in vec3 color;

out vec4 final_color;

void main() {
    final_color = vec4(color, 1.0);
    final_color.r += abs(sin(time));
    final_color.g += abs(cos(time));
    final_color.b += abs(sin(time) * cos(time));
}