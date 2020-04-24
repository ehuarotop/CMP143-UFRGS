#version 330 core

//in vec4 rasterizer_color;
uniform vec4 rasterizer_color;
out vec4 FragColor;

void main(){
	FragColor = rasterizer_color;
}