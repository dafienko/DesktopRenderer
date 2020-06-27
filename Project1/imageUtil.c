#include "assetLoader.h"

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