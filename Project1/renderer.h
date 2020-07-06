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

void set_current_skybox(GLuint skyboxTexture);
void set_background_color(float r, float g, float b);

void set_fov(const float fov);
float get_fov();



#endif