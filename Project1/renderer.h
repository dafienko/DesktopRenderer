#ifndef RENDERER_H
#define RENDERER_H

#include "glExtensions.h"
#include "vectorMath.h"
#include <Vfw.h>

typedef struct camera {
	vec3f position;
	vec3f rotation;
} camera;

camera currentCamera;

void init(int width, int height);
void display(HDRAWDIB hdd, HDC hdc, int dWidth, int dHeight, int destPosX, int destPosY);
float draw(int dWidth, int dHeight);
void end();

void use_rc(HDC*, HGLRC*);


#define MAX_POINTLIGHTS_PER_OBJECT 4 //number of pointlights that can affect one object at a time
int maxPointLights; // number of total pointlights that can exist at one time
typedef struct pointlight {
	vec4f position;
	vec4f color;
	float intensity;
	float range;
	vec2f padding;
} pointlight;
pointlight** pointlights;

void buffer_pointlight_data(vec4f position);

void set_fov(const float fov);
float get_fov();

#endif