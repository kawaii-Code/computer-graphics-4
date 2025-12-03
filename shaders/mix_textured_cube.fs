#version 330

in vec3 color;
in vec2 texcoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float mixRatio; 

out vec4 FragColor;

void main() {
    vec4 tex1Color = texture(texture1, texcoord);
    vec4 tex2Color = texture(texture2, texcoord);
    
    vec4 mixedColor = mix(tex1Color, tex2Color, mixRatio);
    
    FragColor = mixedColor; //* vec4(color, 1.0f);
}