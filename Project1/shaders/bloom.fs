#version 430

in vec2 pos;
out vec4 color;

uniform int msaa;
uniform int numSamples;

layout (binding=2) uniform sampler2D btex;

layout (binding=3) uniform sampler2D tex;
layout (binding=3) uniform sampler2DMS texms;

uniform int sWidth, sHeight, sampleNum;
uniform float offsetScale;

void main() {
	vec2 texPos = vec2((pos.x + 1) / 2,  (pos.y + 1) / 2);
	
	float mid = (sampleNum - 1.0f) / 2.0f;
	float totalWeight = 0;
	
	float maxDistFromMid = length(vec2(mid, mid));
	for (int y = 0; y < sampleNum; y++) {
		for (int x = 0; x < sampleNum; x++) {
			vec2 offset = vec2(y - mid, x - mid);
			offset = vec2(offset.x * (1.0f / sWidth), offset.y * (1.0f / sHeight));
			
			float weight = (maxDistFromMid - length(offset)) / maxDistFromMid;
			color += texture(btex, (texPos + offset * offsetScale)) * weight;
			totalWeight += weight;
		}
	}
	
	color /= totalWeight;
	
	///*
	if (msaa == 0) {
		color += texture(tex, texPos);
	} else {
		texPos = vec2((pos.x + 1) / 2,  (pos.y + 1) / 2) * vec2(sWidth, sHeight);
		vec4 texelColor = vec4(0);
		for (int i = 0; i < numSamples; i++) {
			texelColor += texelFetch(texms, ivec2(texPos.x, texPos.y), i);
		}
		color += texelColor / numSamples;
	}
	//*/
	
}




















