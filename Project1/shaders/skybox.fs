#version 430

out vec4 FragColor;

in vec3 TexCoords;

layout (binding=0) uniform samplerCube skybox;

void main()
{    
    FragColor = texture(skybox, TexCoords);
	//FragColor = vec4(TexCoords, 1.0f);
}