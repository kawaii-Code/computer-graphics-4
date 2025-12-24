#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

uniform sampler2D heightmap;
uniform float heightScale;
uniform float terrainSize;   // world size

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main()
{
    float h = texture(heightmap, aUV).r * heightScale;

    vec3 pos = aPos;
    pos.y = h;

    float texel = 1.0 / textureSize(heightmap, 0).x;

    float hL = texture(heightmap, aUV + vec2(-texel, 0)).r * heightScale;
    float hR = texture(heightmap, aUV + vec2( texel, 0)).r * heightScale;
    float hD = texture(heightmap, aUV + vec2(0, -texel)).r * heightScale;
    float hU = texture(heightmap, aUV + vec2(0,  texel)).r * heightScale;

    vec3 dx = vec3(terrainSize * texel, hR - hL, 0.0);
    vec3 dz = vec3(0.0, hU - hD, terrainSize * texel);

    Normal = normalize(cross(dz, dx));

    FragPos = pos;
    TexCoord = aUV;

    gl_Position = proj * view * vec4(pos, 1.0);
}
