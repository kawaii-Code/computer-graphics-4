#version 330

layout(location = 0) in vec3 aPos;      
layout(location = 1) in vec3 aColor;    
layout(location = 2) in vec2 aTexCoord

uniform float rotation_x;  
uniform float rotation_y;   
uniform float rotation_z;
uniform vec2 position;
uniform vec3 scale;

out vec3 color;
out vec2 texcoord;

void main() {
    mat4 scale_mat = mat4(
        scale.x, 0.0, 0.0, 0.0,
        0.0, scale.y, 0.0, 0.0,
        0.0, 0.0, scale.z, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    
    mat4 rot_x = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, cos(rotation_x), -sin(rotation_x), 0.0,
        0.0, sin(rotation_x), cos(rotation_x), 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    
    mat4 rot_y = mat4(
        cos(rotation_y), 0.0, sin(rotation_y), 0.0,
        0.0, 1.0, 0.0, 0.0,
        -sin(rotation_y), 0.0, cos(rotation_y), 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    
    mat4 rot_z = mat4(
        cos(rotation_z), -sin(rotation_z), 0.0, 0.0,
        sin(rotation_z), cos(rotation_z), 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    
    
    mat4 rotation = rot_z * rot_y * rot_x;
    vec4 pos = rotation * scale_mat * vec4(aPos, 1.0);

    pos.x += position.x;
    pos.y += position.y;

    gl_Position = pos;
    color = aColor;
    texcoord = aTexCoord;
}