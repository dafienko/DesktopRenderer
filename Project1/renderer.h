#ifndef RENDERER_H
#define RENDERER_H

#include "glExtensions.h"
#include "vectorMath.h"
#include <Vfw.h>

#define CHECK_GL_ERRORS check_gl_err(__FILE__, __LINE__)

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
void check_gl_err(const char*, const int);

void use_rc(HDC*, HGLRC*);


#endif