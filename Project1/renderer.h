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

void init();
void display(HDRAWDIB hdd, HDC hdc, int dWidth, int dHeight);
float draw(int dWidth, int dHeight);
void end();

void resize(int width, int height);

void use_rc(HDC*, HGLRC*);


#endif