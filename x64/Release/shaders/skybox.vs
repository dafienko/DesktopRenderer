#version 430

layout (location=0) in vec3 position;

uniform mat4 perspectiveMatrix;
uniform mat4 mvMatrix;

uniform vec3 scale;

out vec3 TexCoords;

void main()
{
    TexCoords = position;
    gl_Position = perspectiveMatrix * mvMatrix * vec4(position * scale, 1.0);
}  