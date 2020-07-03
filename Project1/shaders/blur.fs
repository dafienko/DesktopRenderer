#version 430

in vec2 pos;
out vec4 color;

layout (binding=1) uniform sampler2D tex;

uniform int sWidth, sHeight;

uniform float scale;
uniform int sampleNum;

void main() {

	vec2 texPos = vec2((pos.x + 1) / 2,  (pos.y + 1) / 2);
	
	float mid = (sampleNum - 1.0f) / 2.0f;
	for (int y = 0; y < sampleNum; y++) {
		for (int x = 0; x < sampleNum; x++) {
			vec2 offset = vec2(y - mid, x - mid);
			offset = vec2(offset.x * (1.0f / sWidth), offset.y * (1.0f / sHeight));
			
			color += texture(tex, (texPos + offset));
		}
	}
	
	color /= (sampleNum * sampleNum);
}




















