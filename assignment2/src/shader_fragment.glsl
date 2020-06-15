#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec3 FinalColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec4 objectColor;

#define NORMAL 0
#define GOURAUD_AD 1
#define GOURAUD_ADS 2
#define PHONG 3

uniform int shading_type;

void main(){

	if(shading_type == NORMAL){
		FragColor = objectColor;
	} else if(shading_type == GOURAUD_AD){
		FragColor = objectColor * vec4(FinalColor, 1.0);
	} else if(shading_type == GOURAUD_ADS){
		FragColor = objectColor * vec4(FinalColor, 1.0);
	}else if(shading_type == PHONG){
		//ambient light
		float const_ambient = 0.5;
		vec3 ambient_light = vec3(const_ambient * lightColor);

		//diffuse light
		vec3 norm = normalize(Normal);
		vec3 lightDir = normalize(lightPos - FragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse_light = diff * lightColor;

		//specular light
		float specularStrength = 0.5;
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular_light = specularStrength * spec * lightColor;
		
		FragColor = objectColor * (vec4(ambient_light, 1.0f) + vec4(diffuse_light, 1.0f) + vec4(specular_light, 1.0f));
	}
}