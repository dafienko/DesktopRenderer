#ifndef ASSETLOADER_H
#define ASSETLOADER_H

#include "glExtensions.h"
#include "matrixMath.h"
#include <gl/GL.h>

typedef struct {
	int numLines;
	char** lines;
	int* lengths;
} lines_data;

lines_data get_file_lines(const char* filename);
void free_lines_data(lines_data* ld);

GLuint create_vertex_shader(const char* shaderName);
GLuint create_fragment_shader(const char* shaderName);
GLuint create_program(GLuint* shaders, int numShaders);


typedef struct {
	int* indices;
	vec3f* normals;
	vec3f* positions;
	unsigned int numFaces;
	unsigned int numIndices;
} obj_data;

obj_data get_obj_data(const char* filename);
void free_obj_data(obj_data* od);

typedef struct {
	GLuint* vao;
	GLuint* vbo;
	GLuint hProgram; 
	GLuint numFaces;
	vec3f position, rotation, scale;
} drawable;

drawable obj_to_drawable(obj_data* od);
void drawable_draw(drawable* d, mat4f perspectiveMatrix, mat4f cameraMatrix, GLuint skyboxTexture);

typedef struct {
	unsigned int width;
	unsigned int height;
	char* lpBits;
} image_bit_data;

image_bit_data read_png_file(const char* filename);
image_bit_data read_png_file_simple(const char* filename);
void free_image_bit_data(image_bit_data* ibd);

image_bit_data get_image_rect(const image_bit_data src, int x, int y, int cx, int cy);

GLuint create_skybox_texture(const char* filename);

#endif