#version 430

uniform vec3 cameraPos;

in vec3 norm;
in vec3 worldPos;

out vec4 color;

layout (binding=0) uniform samplerCube skybox;

float PI = 3.14159265359;
vec3 lightDir = normalize(vec3(-1, -1, 1));

struct material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
	float k;
};

uniform int numPointLights;
layout (std140, binding = 1) uniform lightblock {
	vec4 lightIntensity;
	vec4 lightColor;
	float intensity;
	float range;
	vec2 padding;
};

vec3 pointlightPos = vec3(0, -4, -4);
vec4 pointlightColor = vec4(1, 0, 1, 1);
float pointlightRange = 6;
float pointlightIntensity = 0;


vec3 camDir = normalize(worldPos - cameraPos); // direction vector from camera to position
float lightAngle = max(0, dot(-lightDir, norm)); 
float camAngle = acos(min(dot(norm, camDir), 1));


vec4 phong_shade(material m, vec3 lightPos) {
	vec3 color = vec3(0);
	
	float diffuse = lightAngle;

	float specular = 0;
	if (dot(-lightDir, norm) > 0) {
		vec3 v = -camDir;
		specular = dot(v, reflect(lightDir, norm));
		specular = max(0, specular);
		specular = pow(specular, m.shininess);
		specular = min(1, max(0, m.k * specular));
	}
	
	color = m.ambient + diffuse * m.diffuse + specular * m.specular;
	
	return vec4(min(1, color.x), min(1, color.y), min(1, color.z), 1);
}


void main(void) {
	material m;
	m.ambient = vec3(0, 0, 0);
	m.diffuse = vec3(0.3, 0.3, 0.3);
	m.specular = vec3(1, 1, 0);
	m.k = .02;
	m.shininess = 32;
	
	color = phong_shade(m, -lightDir);
	
	vec3 bounce = reflect(camDir, norm);
	vec4 reflectColor = texture(skybox, bounce);
	
	float bias = 0;
	float scale = 4;
	float p = 5;
	float R = max(0, min(1, (scale * pow(1 - abs(dot(camDir, norm)), p))));
	
	color = (reflectColor * (R) + color * (1-R));

	vec3 pointlightVec = worldPos - pointlightPos;
	float dist = length(pointlightVec);
	vec4 lColor = vec4(0);
	if (dist < pointlightRange && dot(normalize(pointlightVec), norm) < 0) {
		float alpha = 1 - (dist / pointlightRange); // 1 is right next to the light, 0 is really far away
		
		alpha = pow(alpha, 2);
		alpha = alpha * pointlightIntensity;
		alpha = max(0, min(1, alpha));
		lColor = alpha * pointlightColor;
	}
	
	color += lColor;
}






















