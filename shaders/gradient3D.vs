#version 330

in vec3 vertex_position;
in vec3 vertex_color;

//uniform float rotation_x;  
//uniform float rotation_y;   
//uniform float rotation_z;
//uniform vec2 position;
//uniform float zoom;
//uniform vec3 scale;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

out vec3 color;

void main() {
    //mat4 scale_mat = mat4(
    //    scale.x, 0.0, 0.0, 0.0,
    //    0.0, scale.y, 0.0, 0.0,
    //    0.0, 0.0, scale.z, 0.0,
    //    0.0, 0.0, 0.0, 1.0
    //);

    //mat4 rot_x = mat4(
    //    1.0, 0.0, 0.0, 0.0,
    //    0.0, cos(rotation_x), -sin(rotation_x), 0.0,
    //    0.0, sin(rotation_x), cos(rotation_x), 0.0,
    //    0.0, 0.0, 0.0, 1.0
    //);
    //
    //mat4 rot_y = mat4(
    //    cos(rotation_y), 0.0, sin(rotation_y), 0.0,
    //    0.0, 1.0, 0.0, 0.0,
    //    -sin(rotation_y), 0.0, cos(rotation_y), 0.0,
    //    0.0, 0.0, 0.0, 1.0
    //);
    //
    //mat4 rot_z = mat4(
    //    cos(rotation_z), -sin(rotation_z), 0.0, 0.0,
    //    sin(rotation_z), cos(rotation_z), 0.0, 0.0,
    //    0.0, 0.0, 1.0, 0.0,
    //    0.0, 0.0, 0.0, 1.0
    //);
    //
    //mat4 rotation = rot_z * rot_y * rot_x;
    //
    //vec4 pos = rotation * scale_mat * vec4(vertex_position, 1.0);

    //pos.xyz *= (1.0 + zoom);
    //pos.x += position.x;
    //pos.y += position.y;

    mat4 mvp = world * view * proj;
    gl_Position = mvp * vec4(vertex_position, 1.0);
    color = vertex_color;
}
