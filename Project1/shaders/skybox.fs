#version 430

out vec4 color;

in vec3 TexCoords;

layout (binding=0) uniform samplerCube skybox;

vec3 globalLightDir = vec3(1, -1, 0);

void main()
{    
    color = texture(skybox, TexCoords);
	
	float sunAngle = max(0, dot(normalize(-globalLightDir), normalize(TexCoords)));
	sunAngle = pow(sunAngle, 32);
	color = mix(color, vec4(1), sunAngle);
}