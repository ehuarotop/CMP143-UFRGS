#version 330

out vec4 outColor;

in vec2 TexCoord;
in float distance_from_camera;

uniform sampler2D texture1;

float gaussian(float x, float sigma) {
    return exp(-x*x / 2 / sigma);
}

void main() {
    float distance_from_center = length(TexCoord.xy - vec2(0.5,0.5));
    float v = gaussian(distance_from_center, 0.05);
    vec4 density_color = vec4(v,v,v,1); // quanto habilitar o blending: vec4(1,1,1,v);
    vec4 image_color = texture(texture1, TexCoord);
    float alpha = clamp(2 * gaussian(distance_from_camera, 10.0), 0, 1); // TODO: talvez encontrar valor melhor
    outColor = mix(density_color, image_color, alpha);
}
