#version 330

layout (location=0) in vec3 position;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 cameraRightWorldSpace;
uniform vec3 cameraUpWorldSpace;
uniform vec3 billboardPosition;
uniform vec2 billboardSize;

void main() {

	vec3 vertexPosition_worldSpace = billboardPosition 
										+ cameraRightWorldSpace * position.x * billboardSize.x
										+ cameraUpWorldSpace * position.y * billboardSize.y;

	gl_Position = projection * view * vec4(vertexPosition_worldSpace);

    //gl_Position = projection * view * model * vec4(position, 1.0f);
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}