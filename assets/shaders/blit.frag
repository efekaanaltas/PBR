#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float exposure;

const float offset = 1.0 / 300.0;  

void main()
{ 
	vec3 color = texture(screenTexture, TexCoords).rgb;
	color = vec3(1.0) - exp(-color * exposure);
	color = pow(color, vec3(1.0/2.2));
	FragColor = vec4(color, 1.0);
}