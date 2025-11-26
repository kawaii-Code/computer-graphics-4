#version 330

in vec2 vertex_position;
in vec3 vertex_color;

out vec3 color;

void main() {
    gl_Position = vec4(vertex_position, 0.0, 1.0);
    color = vertex_color;
}
