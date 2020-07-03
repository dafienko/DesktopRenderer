#version 430

layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;

uniform mat4 perspectiveMatrix;
uniform mat4 mvMatrix;
uniform mat4 mMatrix;

uniform vec3 scale;
uniform vec3 cameraPos;

out vec3 norm;
out vec3 worldPos;

vec3 lightDir = normalize(vec3(-1, -1, -1));

void main(void) {
	norm = (mMatrix * vec4(normal, 0)).xyz;
	//norm = normal;
	worldPos = (mMatrix * vec4(position, 1)).xyz; // world position
	
	gl_Position = perspectiveMatrix * mvMatrix * vec4(position * scale, 1.0);
}
