#version 330

in vec3 vertex_position;
in vec3 vertex_color;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

out vec3 color;

void main() {
    mat4 mvp = proj * view * world;
    gl_Position = mvp * vec4(vertex_position, 1.0);
    color = vertex_color;
}
