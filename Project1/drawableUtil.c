#include "assetLoader.h"
#include "glExtensions.h"
#include "renderer.h"
#include <stdio.h>

drawable obj_to_drawable(obj_data* od) {
	drawable d = { 0 };
	d.vao = calloc(1, sizeof(GLuint));
	d.vbo = calloc(3, sizeof(GLuint));

	glGenVertexArrays(1, d.vao);
	glBindVertexArray(*(d.vao + 0));

	glGenBuffers(3, d.vbo);

	/* buffer positions */
	glBindBuffer(GL_ARRAY_BUFFER, *(d.vbo + 0));
	int size = sizeof(vec3f) * od->numIndices;
	glBufferData(GL_ARRAY_BUFFER, size, od->positions, GL_STATIC_DRAW);

	/* buffer normals */
	glBindBuffer(GL_ARRAY_BUFFER, *(d.vbo + 1));
	OutputDebugStringA("Buffering normals:\n");
	size = sizeof(vec3f) * od->numIndices;
	for (int i = 0; i < od->numIndices; i++) {
		vec3f n = *(od->normals + i);
		int h = 0;
	}
	glBufferData(GL_ARRAY_BUFFER, size, od->normals, GL_STATIC_DRAW);

	/* buffer indices */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *(d.vbo + 2));
	size = sizeof(*(od->indices + 0)) * od->numFaces * 3;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, od->indices, GL_STATIC_DRAW);

	d.numFaces = od->numFaces;

	return d;
}



void drawable_draw(drawable* d, mat4f perspectiveMatrix, mat4f cameraMatrix, GLuint skyboxTexture) {
	static GLuint perspectiveLoc, mvLoc, mLoc, cposLoc, scaleLoc,  /* random bullcrap */
		maoLoc, mAlbedoLoc, mMetallicLoc, mRoughnessLoc; /* material bullcrap */

	glUseProgram(d->hProgram);

	//buffer_pointlight_data((vec4f) {d->position.x, d->position.y, d->position.z, 0.0f});

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
	maoLoc = glGetUniformLocation(d->hProgram, "m.ao");
	mAlbedoLoc = glGetUniformLocation(d->hProgram, "m.albedo");
	mMetallicLoc = glGetUniformLocation(d->hProgram, "m.metallic");
	mRoughnessLoc = glGetUniformLocation(d->hProgram, "m.roughness");
	
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
	
	glUniform3f(cposLoc, currentCamera.position.x, currentCamera.position.y, currentCamera.position.z);
	glUniform3f(scaleLoc, d->scale.x, d->scale.y, d->scale.z);

	glUniform3f(mAlbedoLoc, d->material.color.x, d->material.color.y, d->material.color.z);
	glUniform1f(maoLoc, d->material.ao);
	glUniform1f(mMetallicLoc, d->material.metallic);
	glUniform1f(mRoughnessLoc, d->material.roughness);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *(d->vbo + 2));

	glDrawElements(GL_TRIANGLES, d->numFaces * 3, GL_UNSIGNED_INT, NULL);
}