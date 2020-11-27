#version 430 

in vec2 texPos;
in vec4 glPos;

layout (binding=0) uniform sampler2D tex;

uniform int particleAlive;
uniform int renderColor;

vec3 fogColor = vec3(0);
float fogStart = 0;
float fogEnd = 2000;

out vec4 FragColor;

void main() {
	vec4 texColor = texture(tex, texPos);
	float alpha = texColor.a;
	
	if (particleAlive != 0) {
		if (renderColor != 0) {
			FragColor = texColor;
			FragColor.a = alpha;
			
			float fogAlpha = max(0, min(1, (glPos.z - fogStart) / (fogEnd - fogStart)));
			FragColor = mix(FragColor, vec4(fogColor, alpha), fogAlpha);
		} else {
			FragColor = vec4(0, 0, 0, alpha);
		}
	}
}