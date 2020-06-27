#version 430

uniform vec3 cameraPos;

in vec3 norm;
in vec3 worldPos;

out vec4 color;

layout (binding=0) uniform samplerCube skybox;

float PI = 3.14159265359;
vec3 lightDir = normalize(vec3(-1, -2, -1.5));
float SHININESS = 16;
vec4 AMBIENT = vec4(0, .2, .2, 0);

void main(void) {
	vec3 camDir = normalize(worldPos - cameraPos); // direction vector from camera to position
	float lightAngle = max(0, dot(-lightDir, norm)); 
	float camAngle = acos(min(dot(norm, camDir), 1));
	
	float diffuse = lightAngle;

	
	float specular = 0;
	if (dot(-lightDir, norm) > 0); {
		vec3 v = (cameraPos - worldPos);
		vec3 h = normalize(lightDir + v);
		float specAngle = max(0, dot(h, reflect(lightDir, norm)));
		specular = pow(specAngle, SHININESS); 
	}
	
	color = AMBIENT + vec4(specular, min(diffuse + specular, 1), min(diffuse + specular, 1), 1.0f);
	
	vec3 bounce = reflect(camDir, norm);
	color = texture(skybox, bounce);
	//color = vec4(norm, 1.0f);
}
