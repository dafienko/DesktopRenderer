#include "renderer.h"
#include "assetLoader.h"

void draw_model(object_model* om, drawable* d, mat4f perspectiveMatrix, mat4f cameraMatrix, GLuint skyboxTexture) {
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

	mat4f mMatrix = from_position_and_rotation(om->position, om->rotation);
	vals = get_vals_mat4f(mMatrix);
	glUniformMatrix4fv(mLoc, 1, GL_FALSE, vals);
	free(vals);

	mat4f mvMatrix = mat_mul_mat(cameraMatrix, mMatrix);
	vals = get_vals_mat4f(mvMatrix);
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, vals);
	free(vals);

	glUniform1f(threshLoc, d->bloomThreshold);

	glUniform3f(cposLoc, currentCamera.position.x, currentCamera.position.y, currentCamera.position.z);
	glUniform3f(scaleLoc, om->scale.x, om->scale.y, om->scale.z);

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