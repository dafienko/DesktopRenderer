#include "assetLoader.h"
#include "glExtensions.h"
#include "renderer.h"
#include <stdio.h>
#include "errors.h"

drawable obj_to_drawable(obj_data* od) {
	drawable d = { 0 };
	d.vao = calloc(1, sizeof(GLuint));
	d.vbo = calloc(3, sizeof(GLuint));

	// copy the obj's material library
	mtllib mlib = { 0 };
	mlib.numMaterials = od->materials.numMaterials;
	mlib.materials = calloc(mlib.numMaterials, sizeof(mtllib_material));
	memcpy(mlib.materials, od->materials.materials, mlib.numMaterials * sizeof(mtllib_material));
	
	for (int i = 0; i < mlib.numMaterials; i++) {
		char* mtlNameSrc = (od->materials.materials + i)->materialName;
		int nameLen = strlen(mtlNameSrc);
		(mlib.materials + i)->materialName = calloc(nameLen + 1, sizeof(char));
		memcpy((mlib.materials + i)->materialName, mtlNameSrc, nameLen);
	}
	d.materials = mlib;
	d.numMaterials = mlib.numMaterials;

	// copy the obj's material-group bounds data
	d.materialBounds = calloc(mlib.numMaterials + 1, sizeof(int));
	memcpy(d.materialBounds, od->materialBounds, (mlib.numMaterials + 1) * sizeof(int));

	glGenVertexArrays(1, d.vao);
	CHECK_GL_ERRORS;
	glBindVertexArray(*(d.vao + 0));
	CHECK_GL_ERRORS;

	glGenBuffers(3, d.vbo);
	CHECK_GL_ERRORS;

	/* buffer positions */
	glBindBuffer(GL_ARRAY_BUFFER, *(d.vbo + 0));
	CHECK_GL_ERRORS;
	int size = sizeof(vec3f) * od->numUniqueIndices;
	glBufferData(GL_ARRAY_BUFFER, size, od->positions, GL_STATIC_DRAW);
	CHECK_GL_ERRORS;

	/* buffer normals */
	glBindBuffer(GL_ARRAY_BUFFER, *(d.vbo + 1));
	CHECK_GL_ERRORS;
	size = sizeof(vec3f) * od->numUniqueIndices;
	glBufferData(GL_ARRAY_BUFFER, size, od->normals, GL_STATIC_DRAW);
	CHECK_GL_ERRORS;

	/* buffer indices */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *(d.vbo + 2));
	CHECK_GL_ERRORS;
	size = sizeof(*(od->indices + 0)) * od->numTris * 3;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, od->indices, GL_STATIC_DRAW);
	CHECK_GL_ERRORS;

	d.drawType = GL_TRIANGLES;

	return d;
}

void free_drawable(drawable* drawable) {
	glDeleteBuffers(3, drawable->vbo);
	glDeleteVertexArrays(1, drawable->vao);
	free(drawable->vbo);
	free(drawable->vao);
	free_mtllib_data(&drawable->materials);
}