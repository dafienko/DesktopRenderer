#version 430

layout (location=0) in vec2 position;

out vec2 pos;

void main(void) {
	pos = position;
	
	gl_Position = vec4(position.xy, 0, 1);
}
