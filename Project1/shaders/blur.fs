#version 430

layout (binding=4) uniform sampler2D tex;

uniform int sWidth, sHeight, sampleNum;
uniform vec2 blurDir;
uniform float offsetScale;

in vec2 pos;
out vec4 color;

const float c = .9f;

float Gaussian(float x) {
	return exp(-(x * x)/(.5 * c * c));
}

void main() {
	vec2 texPos = vec2((pos.x + 1) / 2,  (pos.y + 1) / 2);
	vec2 dir = vec2(blurDir.x / sWidth, blurDir.y / sHeight);
	
	float totalWeight = Gaussian(0);
	vec4 acc = texture(tex, texPos) * totalWeight;
	for (int i = 1; i < sampleNum; i++) {
		float alpha = i;
		alpha /= sampleNum;
		float weight = Gaussian(alpha);
		
		acc += texture(tex, texPos + dir * i * offsetScale) * weight;
		acc += texture(tex, texPos + dir * -i * offsetScale) * weight;
		
		totalWeight += weight * 2;
	}
	
	acc /= totalWeight;
	color = vec4(acc.xyz, 1);
}




















