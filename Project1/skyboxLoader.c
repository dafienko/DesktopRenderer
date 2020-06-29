#include "assetLoader.h"
#include "errors.h"
#include <math.h>

GLuint create_skybox_texture(const char* filename) {
	image_bit_data ibd = read_png_file_simple(filename);
	GLuint skybox;

	int skyboxFaceSize = floor(ibd.width / 4);
	glActiveTexture(GL_TEXTURE0);
	CHECK_GL_ERRORS;
	glGenTextures(1, &skybox);
	CHECK_GL_ERRORS;
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
	CHECK_GL_ERRORS;

	image_bit_data top = get_image_rect(ibd, skyboxFaceSize * 1, skyboxFaceSize * 0, skyboxFaceSize, skyboxFaceSize);
	image_bit_data bottom = get_image_rect(ibd, skyboxFaceSize * 1, skyboxFaceSize * 2, skyboxFaceSize, skyboxFaceSize);
	image_bit_data back = get_image_rect(ibd, skyboxFaceSize * 3, skyboxFaceSize * 1, skyboxFaceSize, skyboxFaceSize);
	image_bit_data left = get_image_rect(ibd, skyboxFaceSize * 0, skyboxFaceSize * 1, skyboxFaceSize, skyboxFaceSize);
	image_bit_data front = get_image_rect(ibd, skyboxFaceSize * 1, skyboxFaceSize * 1, skyboxFaceSize, skyboxFaceSize);
	image_bit_data right = get_image_rect(ibd, skyboxFaceSize * 2, skyboxFaceSize * 1, skyboxFaceSize, skyboxFaceSize);

	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB8, skyboxFaceSize, skyboxFaceSize);

	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, 0, 0,
		skyboxFaceSize, skyboxFaceSize,
		GL_BGRA, GL_UNSIGNED_BYTE,
		top.lpBits);
	CHECK_GL_ERRORS;

	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, 0, 0,
		skyboxFaceSize, skyboxFaceSize,
		GL_BGRA, GL_UNSIGNED_BYTE,
		bottom.lpBits);
	CHECK_GL_ERRORS;

	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0,
		skyboxFaceSize, skyboxFaceSize,
		GL_BGRA, GL_UNSIGNED_BYTE,
		right.lpBits);
	CHECK_GL_ERRORS;

	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, 0, 0,
		skyboxFaceSize, skyboxFaceSize,
		GL_BGRA, GL_UNSIGNED_BYTE,
		left.lpBits);
	CHECK_GL_ERRORS;

	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, 0, 0,
		skyboxFaceSize, skyboxFaceSize,
		GL_BGRA, GL_UNSIGNED_BYTE,
		front.lpBits);
	CHECK_GL_ERRORS;

	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, 0, 0,
		skyboxFaceSize, skyboxFaceSize,
		GL_BGRA, GL_UNSIGNED_BYTE,
		back.lpBits);
	CHECK_GL_ERRORS;

	CHECK_GL_ERRORS;
	CHECK_GL_ERRORS;
	CHECK_GL_ERRORS;

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	CHECK_GL_ERRORS;

	/*
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		0,
		GL_RGBA,
		skyboxFaceSize, skyboxFaceSize,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		top.lpBits);
	CHECK_GL_ERRORS;

	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		0,
		GL_RGBA,
		skyboxFaceSize, skyboxFaceSize,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		bottom.lpBits);
	CHECK_GL_ERRORS;

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		0,
		GL_RGBA,
		skyboxFaceSize, skyboxFaceSize,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		back.lpBits);
	CHECK_GL_ERRORS;

	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		0,
		GL_RGBA,
		skyboxFaceSize, skyboxFaceSize,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		front.lpBits);
	CHECK_GL_ERRORS;

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		0,
		GL_RGBA,
		skyboxFaceSize, skyboxFaceSize,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		right.lpBits);
	CHECK_GL_ERRORS;

	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		0,
		GL_RGBA,
		skyboxFaceSize, skyboxFaceSize,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		left.lpBits);
	CHECK_GL_ERRORS;
	*/

	free_image_bit_data(&top);
	free_image_bit_data(&bottom);
	free_image_bit_data(&back);
	free_image_bit_data(&left);
	free_image_bit_data(&front);
	free_image_bit_data(&right);

	free_image_bit_data(&ibd);

	return skybox;
}