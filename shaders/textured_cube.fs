#version 330

in vec3 color;
in vec2 texcoord;

uniform sampler2D ourTexture;
uniform vec3 colorBoost;

out vec4 FragColor;

void main() {
    vec4 texColor = texture(ourTexture, texcoord);
    
    vec3 boostedColor = texColor.rgb * (1.0 + colorBoost);
    
    FragColor = vec4(boostedColor * color, texColor.a);
}