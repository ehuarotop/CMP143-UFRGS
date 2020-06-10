#version 330 core

layout (location = 0) in vec2 vert;

void main(){
    gl_Position = vec4(vert, 1.0f, 1.0f);
}