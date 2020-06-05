#version 330 core

uniform vec4 rasterizer_color;
out vec4 FragColor;

void main(){
	FragColor = rasterizer_color;
}