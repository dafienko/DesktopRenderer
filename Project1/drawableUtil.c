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

int timesDrawn = 0;
void drawable_draw(drawable* d, mat4f perspectiveMatrix, mat4f cameraMatrix, GLuint skyboxTexture) {
	static GLuint perspectiveLoc, mvLoc, mLoc, cposLoc, scaleLoc, threshLoc,  /* random bullcrap */
		maLoc, mdLoc, msLoc, mshiLoc, mkLoc, eLoc; /* material bullcrap */

	glUseProgram(d->hProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

	glBindVertexArray(*(d->vao + 0));

	glBindBuffer(GL_ARRAY_BUFFER, *(d->vbo + 1));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, *(d->vbo + 0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	perspectiveLoc = glGetUniformLocation(d->hProgram, "perspectiveMatrix");
	mLoc = glGetUniformLocation(d->hProgram, "mMatrix");
	mvLoc = glGetUniformLocation(d->hProgram, "mvMatrix");
	cposLoc = glGetUniformLocation(d->hProgram, "cameraPos");
	scaleLoc = glGetUniformLocation(d->hProgram, "scale");
	threshLoc = glGetUniformLocation(d->hProgram, "threshold");

	maLoc = glGetUniformLocation(d->hProgram, "m.ambient");
	mdLoc = glGetUniformLocation(d->hProgram, "m.diffuse");
	msLoc = glGetUniformLocation(d->hProgram, "m.specular");
	mshiLoc = glGetUniformLocation(d->hProgram, "m.shininess");
	mkLoc = glGetUniformLocation(d->hProgram, "m.k");
	eLoc = glGetUniformLocation(d->hProgram, "emitter");

	float* vals = get_vals_mat4f(perspectiveMatrix);
	glUniformMatrix4fv(perspectiveLoc, 1, GL_FALSE, vals);
	free(vals);

	mat4f mMatrix = from_position_and_rotation(d->position, d->rotation);
	vals = get_vals_mat4f(mMatrix);
	glUniformMatrix4fv(mLoc, 1, GL_FALSE, vals);
	free(vals);

	mat4f mvMatrix = mat_mul_mat(cameraMatrix, mMatrix);
	vals = get_vals_mat4f(mvMatrix);
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, vals);
	free(vals);

	glUniform1f(threshLoc, d->bloomThreshold);

	glUniform3f(cposLoc, currentCamera.position.x, currentCamera.position.y, currentCamera.position.z);
	glUniform3f(scaleLoc, d->scale.x, d->scale.y, d->scale.z);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *(d->vbo + 2));

	for (int i = 0; i < d->numMaterials; i++) {
		int indexFloor = *(d->materialBounds + i);
		int indexCeil = *(d->materialBounds + i + 1);
		int groupNumIndices = indexCeil - indexFloor;

		mtllib_material mtl = *(d->materials.materials + i);

		glUniform3f(maLoc, mtl.material.ambient.x, mtl.material.ambient.y, mtl.material.ambient.z);
		glUniform3f(mdLoc, mtl.material.diffuse.x, mtl.material.diffuse.y, mtl.material.diffuse.z);
		glUniform3f(msLoc, mtl.material.specular.x, mtl.material.specular.y, mtl.material.specular.z);
		glUniform1f(mshiLoc, 1);
		glUniform1f(mkLoc, 1.0f);
		glUniform1i(eLoc, mtl.material.emitter);

		glDrawRangeElements(d->drawType, indexFloor, indexCeil, groupNumIndices, GL_UNSIGNED_INT, indexFloor * sizeof(int));
	}
}

void free_drawable(drawable* drawable) {
	glDeleteBuffers(3, drawable->vbo);
	glDeleteVertexArrays(1, drawable->vao);
	free(drawable->vbo);
	free(drawable->vao);
	free_mtllib_data(&drawable->materials);
}