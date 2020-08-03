#version 330

out vec4 outColor;

in vec2 TexCoord;
in float distance_from_camera;

uniform sampler2D texture1;

float gaussian(float x, float sigma) {
    return exp(-x*x / 2 / sigma);
}

void main() {
	//calculating distance from the center
    float distance_from_center = length(TexCoord.xy - vec2(0.5,0.5));

    //calculating transparency value for the density color.
    float v = gaussian(distance_from_center, 0.05);

    //Setting the density color as white with a value transparency equal to v.
    vec4 density_color = vec4(1,1,1,0.2*v);

    //Setting component color (applying texture) when the camera is near.
    vec4 image_color = texture(texture1, TexCoord);

    //Getting the weights to be applied to each color in order to obtain the final color
    float alpha = clamp(2 * gaussian(distance_from_camera, 10.0), 0, 1);

    //Blending both colors to produce final color.
    outColor = mix(density_color, image_color, alpha);
}
