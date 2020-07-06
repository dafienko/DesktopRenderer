#ifndef RENDERER_H
#define RENDERER_H

#include "glExtensions.h"
#include "vectorMath.h"
#include "assetLoader.h"
#include <Vfw.h>

typedef int HMESH;

typedef struct camera {
	vec3f position;
	vec3f rotation;
} camera;

typedef struct {
	vec3f position;
	vec3f rotation;
	vec3f scale;

	int visible;

	HMESH hModel;
} object_model;

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

HMESH load_mesh(const char* meshName, const char* mtlFileName);
object_model* create_model_from_mesh(HMESH hMesh);

void draw_model(object_model* om, drawable* d, mat4f perspectiveMatrix, mat4f cameraMatrix, GLuint skyboxTexture);

HMESH init_dynamic_mesh(GLenum drawType);
void edit_mesh_indices(HMESH hMesh, int* indices, int numIndices);
void edit_mesh_positions(HMESH hMesh, vec3f* positions, int numPositions);
void edit_mesh_normals(HMESH hMesh, vec3f* normals, int numNormals);
void edit_mesh_mtl_data(HMESH hMesh, mtllib mlib, int* groupBounds, int numGroups);

#endif