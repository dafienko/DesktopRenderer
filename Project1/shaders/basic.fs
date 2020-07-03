#version 430

uniform vec3 cameraPos;

in vec3 norm;
in vec3 worldPos;

out vec4 color;

layout (binding=0) uniform samplerCube skybox;

struct material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
	float k;
};

uniform material m;

float PI = 3.14159265359;
vec3 globalLightDir = normalize(vec3(1, -1, 0));



vec4 phong_shade(vec3 V, vec3 lightDir, float intensity) {
	vec3 rColor = vec3(0);
	
	float diffuse = max(0, dot(-lightDir, norm));

	float specular = 0;
	if (dot(norm, lightDir) < 0) {
		specular = max(0, dot(reflect(lightDir, norm), -V));
		specular = pow(specular, m.shininess);
		specular = specular * m.k;
		specular = min(specular, 1);
	}
	
	rColor = m.ambient + diffuse * m.diffuse * intensity + specular * m.specular * intensity;
	
	return vec4(min(1, rColor.x), min(1, rColor.y), min(1, rColor.z), 1);
}


void main(void) {
	vec3 camDir = normalize(worldPos - cameraPos); // direction vector from camera to position
	
	color = phong_shade(camDir, globalLightDir, 1);
	
	vec3 bounce = reflect(camDir, norm);
	vec4 reflectColor = (texture(skybox, bounce) + vec4(m.specular, 1)) / 2;
	
	float bias = 0;
	float scale = 4;
	float p = 15;
	float R = max(0, min(1, (scale * pow(1 - abs(dot(camDir, norm)), p))));
	
	color = (1 - R) * color + R * reflectColor;
}






















