#version 430

out vec4 color;

in vec3 TexCoords;

layout (binding=0) uniform samplerCube skybox;

uniform vec3 sunColor;
uniform vec3 globalLightDir;



void main()
{    
    vec4 textureColor = texture(skybox, TexCoords);
	color = textureColor * 2.5;

	/*
	float sunAngle = max(0, dot(normalize(-globalLightDir), normalize(TexCoords)));
	sunAngle = pow(sunAngle, 32);
	
	float sunSize = .8;
	
	if (sunAngle > sunSize) {
		vec4 nextToSunColor = mix(color, vec4(sunColor, 1), sunSize);
		float nextToSunAlpha = (sunAngle - sunSize) / (1 - sunSize);
		color = mix(nextToSunColor, vec4(1), nextToSunAlpha);
	} else {
		color = mix(color, vec4(sunColor, 1), sunAngle);
	}
	*/
}