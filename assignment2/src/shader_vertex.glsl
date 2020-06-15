#version 330 core

layout (location = 0) in vec3 vert;
layout (location = 1) in vec3 vert_normal;

out vec3 FragPos;
out vec3 Normal;
out vec3 FinalColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

#define NORMAL 0
#define GOURAUD_AD 1
#define GOURAUD_ADS 2
#define PHONG 3

uniform int shading_type;

void main(){

	if(shading_type == NORMAL){
		gl_Position = projection * view * model * vec4(vert, 1.0);	
	} else if(shading_type == GOURAUD_AD){
		vec3 position = vec3(model * vec4(vert, 1.0));
		vec3 Normal = mat3(transpose(inverse(model))) * vert_normal;

		//ambient
		float const_ambient = 0.5;
		vec3 ambient_light = vec3(const_ambient * lightColor);

		// diffuse 
	    vec3 norm = normalize(Normal);
	    vec3 lightDir = normalize(lightPos - position);
	    float diff = max(dot(norm, lightDir), 0.0);
	    vec3 diffuse_light = diff * lightColor;

	    FinalColor = ambient_light + diffuse_light;

	    gl_Position = projection * view * model * vec4(vert, 1.0);

	} else if(shading_type == GOURAUD_ADS){
		vec3 position = vec3(model * vec4(vert, 1.0));
		vec3 Normal = mat3(transpose(inverse(model))) * vert_normal;

		//ambient
		float const_ambient = 0.5;
		vec3 ambient_light = vec3(const_ambient * lightColor);

		// diffuse 
	    vec3 norm = normalize(Normal);
	    vec3 lightDir = normalize(lightPos - position);
	    float diff = max(dot(norm, lightDir), 0.0);
	    vec3 diffuse_light = diff * lightColor;

	    // specular
	    float specularStrength = 1.0; // this is set higher to better show the effect of Gouraud shading 
	    vec3 viewDir = normalize(viewPos - position);
	    vec3 reflectDir = reflect(-lightDir, norm);  
	    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	    vec3 specular_light = specularStrength * spec * lightColor;

	    FinalColor = ambient_light + diffuse_light + specular_light;

	    gl_Position = projection * view * model * vec4(vert, 1.0);

	} else if (shading_type == PHONG){
		FragPos = vec3(model * vec4(vert, 1.0));
		Normal = vert_normal;

	    gl_Position = projection * view * model * vec4(vert, 1.0);
	}
}