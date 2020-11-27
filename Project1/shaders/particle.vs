#version 430 

layout (location=0) in vec2 quadPos;
layout (location=1) in vec2 quadCorner;

uniform float scale;
uniform mat4 projMat;

uniform mat4 mvMat[50];

out vec2 texPos;
out vec4 glPos;

void main() {
	texPos = quadCorner;
	gl_Position = projMat * mvMat[gl_InstanceID] * vec4(quadPos.xy * 8.0, 0, 1.0);
	//gl_Position = vec4(quadPos.xy, 0.0, 1.0);
	glPos = gl_Position;
}