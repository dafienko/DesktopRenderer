#version 430

in vec2 pos;
out vec4 color;

uniform int msaa;
uniform float intensity;

layout (binding=2) uniform sampler2D btex;

layout (binding=3) uniform sampler2D tex;
layout (binding=3) uniform sampler2DMS texms;

uniform int sWidth, sHeight;

void main() {
	vec2 texPos = vec2((pos.x + 1) / 2,  (pos.y + 1) / 2);
	color = texture(btex, texPos) * intensity;
	color = vec4(color.xyz, 1);
	
	if (msaa == 0) {
		color += texture(tex, texPos);
	} else {
		texPos = vec2((pos.x + 1) / 2,  (pos.y + 1) / 2) * vec2(sWidth, sHeight);
		vec4 texelColor = vec4(0);
		for (int i = 0; i < msaa; i++) {
			texelColor += texelFetch(texms, ivec2(texPos.x, texPos.y), i);
		}
		color += texelColor / msaa;
	}
}




















