#version 430

struct material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
	float k;
};

uniform int skyboxHandle;
uniform vec3 backgroundColor;
uniform vec3 cameraPos;
uniform material m;
uniform float threshold;
uniform int emitter;
uniform vec3 globalLightDir;

layout (binding=0) uniform samplerCube skybox;

in vec3 norm;
in vec3 worldPos;
in vec4 glPos;

out vec4 color;


const float PI = 3.14159265359;

//vec3 fogColor = vec3(202.0 / 255.0, 238.0 / 255.0, 1);
vec3 fogColor = vec3(0);
float fogStart = 0;
float fogEnd = 2000;

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
	if (emitter == 0) { 
		vec3 camDir = normalize(worldPos - cameraPos); // direction vector from camera to position
	
		color = phong_shade(camDir, normalize(globalLightDir), 1);
		
		vec3 bounce = reflect(camDir, norm);
		vec4 skyboxColor;
		if (skyboxHandle > 0) {
			skyboxColor = texture(skybox, bounce);
		} else {
			skyboxColor = vec4(backgroundColor, 1);
		}
		vec4 reflectColor = (skyboxColor + vec4(m.specular, 1)) / 2;
		
		float bias = 0;
		float scale = 4;
		float p = 32;
		float R = max(0, min(1, (scale * pow(1 - abs(dot(camDir, norm)), p))));
		color = mix(color, reflectColor, R);
		
		color = vec4(clamp(color.x, 0, 1), clamp(color.y, 0, 1), clamp(color.z, 0, 1), 1);
		
		float colorVal = (color.x + color.y + color.z) / 3.0f;
		if (colorVal < threshold) {
			color = vec4(0);
		}
	} else {
		color = vec4(m.ambient + m.diffuse + m.specular, 1.0);
	}
	
	//if (threshold == 0) {
		float fogAlpha = max(0, min(1, (glPos.z - fogStart) / (fogEnd - fogStart)));
		color = mix(color, vec4(fogColor, 1), fogAlpha);
	//}
}






















