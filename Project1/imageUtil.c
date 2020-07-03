#include "assetLoader.h"
#include "errors.h"

image_bit_data get_image_rect(const image_bit_data src, int x, int y, int cx, int cy) {
	image_bit_data ibd = { 0 };


	unsigned char* lpBits = calloc(cx * cy * 4, sizeof(unsigned char));

	for (int sy = 0; sy < cy; sy++) {
		for (int sx = 0; sx < cx; sx++) {
			int srcOffset = ((sy + y) * src.width + (sx + x)) * 4;
			int destOffset = (sy * cx + sx) * 4;

			for (int b = 0; b < 4; b++) {
				unsigned char byte = *(src.lpBits + srcOffset + b);
				*(lpBits + destOffset + b) = byte;
			}
		}
	}

	ibd.width = cx;
	ibd.height = cy;
	ibd.lpBits = lpBits;
	

	return ibd;
}

GLuint create_png_texture(const char* filename) {
	image_bit_data ibd = read_png_file_simple(filename);
	GLuint tex;

	CHECK_GL_ERRORS;
	glGenTextures(1, &tex);
	CHECK_GL_ERRORS;
	glBindTexture(GL_TEXTURE_2D, tex);
	CHECK_GL_ERRORS;


	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ibd.width, ibd.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, ibd.lpBits);
	CHECK_GL_ERRORS;
	free_image_bit_data(&ibd);
	return tex;
}