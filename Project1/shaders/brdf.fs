#version 430
in vec3 worldPos;
in vec3 norm;

out vec4 color;

// material parameters
struct material {
	vec3 albedo;
	float metallic;
	float roughness;
	float ao;
};

uniform material m;
// lights
struct light {
	vec3 color;
	float range;
	float intensity;
};
uniform int numLights;
#define MAX_LIGHTS 4
uniform light lights[MAX_LIGHTS];

vec3 lightPositions[4];
vec3 lightColors[4];

layout (binding=0) uniform samplerCube skybox;

uniform vec3 cameraPos;

const float PI = 3.14159265359;
  
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

float schlick(float angle) {
	float reflectance = (1 - m.roughness) * .1;
	return (reflectance + (1 - reflectance) * pow((1 - angle), 5)) * (1 - m.roughness);
}


void main()
{	
	int width = 2;
	int height = 2;
	float s = 8;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			vec3 p = vec3(x * s - s/2, y * s - s/2, 0);
			lightPositions[y * width + x] = p;
			lightColors[y * width + x] = vec3(1, 1, 1);
		}
	}
	
    vec3 N = normalize(norm);
    vec3 V = normalize(cameraPos - worldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, m.albedo, m.metallic);
	           
    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - worldPos);
        vec3 H = normalize(V + L);
        float distance    = length(lightPositions[i] - worldPos);
        float attenuation = 6.0 / (distance * distance);
        vec3 radiance     = lightColors[i] * attenuation;        
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, m.roughness);        
        float G   = GeometrySmith(N, V, L, m.roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
		
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - m.metallic;	  
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular     = numerator / max(denominator, 0.001);  
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * m.albedo / PI + specular) * radiance * NdotL; 
    }   
  
    vec3 ambient = vec3(0.03) * m.albedo * m.ao;
    vec3 color3 = ambient + Lo;
	
    color3 = color3 / (color3 + vec3(1.0));
    color3 = pow(color3, vec3(1.0/2.2));  
   
   
	float R = schlick(dot(norm, V));       
	vec3 bounce = reflect(-V, norm);
	vec3 bounceColor = texture(skybox, bounce).xyz;
	bounceColor = mix((bounceColor + m.albedo) / 2, bounceColor, 1 - m.metallic);
	color3 = bounceColor * R + color3 * (1 - R);
	
    color = vec4(color3, 1.0);
}  





















