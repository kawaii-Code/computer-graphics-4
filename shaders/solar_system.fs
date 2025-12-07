#version 410 core

in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D texture_sampler;
uniform vec3 planet_color;

void main() {
    vec4 tex_color = texture(texture_sampler, tex_coord);
    frag_color = vec4(tex_color.rgb * planet_color, tex_color.a);
}

