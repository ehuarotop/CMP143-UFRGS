#version 330 core

uniform vec4 rasterizer_color;
uniform sampler2D texture1;
out vec4 FragColor;
in vec2 TexCoord;

void main(){
	FragColor = texture(texture1, TexCoord)*rasterizer_color;
}