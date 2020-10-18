#version 430

uniform vec3 sunColor;
uniform vec3 backgroundColor;
uniform vec3 globalLightDir;

out vec4 color;

in vec3 TexCoords;

void main()
{    
	color = vec4(backgroundColor, 1);
	
	float sunAngle = max(0, dot(normalize(-globalLightDir), normalize(TexCoords)));
	float csAlpha = pow(sunAngle, 8);
	float overrideAlpha = pow(csAlpha, 4);
	color = vec4(1, 0, 0, 1);
	//color = mix(color, vec4(sunColor, 1), csAlpha) + vec4(overrideAlpha, overrideAlpha, overrideAlpha, 1);
}