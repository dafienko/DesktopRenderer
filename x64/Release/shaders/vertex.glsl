#version 430

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;

uniform mat4 perspectiveMatrix;
uniform mat4 mvMatrix;
uniform mat4 mMatrix;

uniform vec3 cameraPos;


out vec4 vertexColor;

void main() {
	vertexColor = vec4(norm, 1.0f);
	gl_Position = perspectiveMatrix * mvMatrix * vec4(pos, 1.0f);
}
