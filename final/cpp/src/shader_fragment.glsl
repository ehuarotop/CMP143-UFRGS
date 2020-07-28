#version 330

out vec4 outColor;

in vec2 TexCoord;

uniform sampler2D texture1;

//texture(texture1, TexCoord);
//vec4(1.0f,0.5f, 0.9f, 1.0f);

void main() {
    outColor = texture(texture1, TexCoord);
}