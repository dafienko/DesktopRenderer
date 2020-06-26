#define _CRT_SECURE_NO_WARNINGS

#include "assetLoader.h"
#include "lpng1637/png.h"
#include <stdio.h>
#include <stdlib.h>
#include "errors.h"

#define uch unsigned char
#define uint unsigned int


image_bit_data read_png_file(char* filename) {
    image_bit_data ibd = { 0 };
    
    png_structp	png_ptr;
    png_infop info_ptr;
    FILE* fp;

    int width = 0;
    int height = 0;
    int bitDepth = 0;
    int colorType = 0;

    fopen_s(&fp, filename, "rb");
    if (fp == NULL) {
        check_std_err(filename, errno);
        return;
    }

    /* check file signature */
    uch sig[8];

    fread(sig, 1, 8, fp);
    if (!png_check_sig(sig, 8)) {
        error("Invalid file signature");
        return;
    }


    /* create png structs */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        error("failed to create png_struct");
        return;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        error("failed to create info_struct");
        return;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    /* get png general information */
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bitDepth, &colorType, NULL, NULL, NULL);

    /* get file background color if it exists */
    uch bgRed = 0;
    uch bgGreen = 0;
    uch bgBlue = 0;
    png_color_16p pBackground = NULL;
    

    if (png_get_bKGD(png_ptr, info_ptr, &pBackground)) {
        if (bitDepth == 16) {
            bgRed = pBackground->red >> 8;
            bgGreen = pBackground->green >> 8;
            bgBlue = pBackground->blue >> 8;
        }
        else if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) {
            if (bitDepth == 1)
                bgRed = bgGreen = bgBlue = pBackground->gray ? 255 : 0;
            else if (bitDepth == 2) /* i.e., max value is 3 */
                bgRed = bgGreen = bgBlue = (255 / 3) * pBackground->gray;
            else /* bitDepth == 4 */ /* i.e., max value is 15 */
                bgRed = bgGreen;
        }
        else {
            bgRed = pBackground->red;
            bgGreen = pBackground->green;
            bgBlue = pBackground->blue;
        }
    }

    char* p;
    double displayExponent = 0;
    if ((p = getenv("SCREEN_GAMMA")) != NULL)
        displayExponent = atof(p);
    else
        displayExponent = 1.0;



    if (colorType == PNG_COLOR_TYPE_PALETTE)
        png_set_expand(png_ptr);
    if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
        png_set_expand(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_expand(png_ptr);


    if (bitDepth == 16)
        png_set_strip_16(png_ptr);
    if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    double gamma;

    if (png_get_gAMA(png_ptr, info_ptr, &gamma))
        png_set_gamma(png_ptr, displayExponent, gamma);




    png_uint_32 i, rowbytes;
    png_bytepp row_pointers = calloc(height, sizeof(png_bytep));

    png_read_update_info(png_ptr, info_ptr);

    rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    int* pChannels = (int)png_get_channels(png_ptr, info_ptr);
    uch* imageData;

    if ( (imageData = (uch*)malloc(rowbytes * height)) == NULL ) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return;
    }

    for (i = 0; i < height; ++i)
        *(row_pointers + i) = imageData + i * rowbytes;

    png_read_image(png_ptr, row_pointers);

    png_read_end(png_ptr, NULL);
    
    ibd.width = width;
    ibd.height = height;
    ibd.lpBits = imageData;

    return ibd;
}

image_bit_data read_png_file_simple(char* filename) {
    png_image image; /* The control structure used by libpng */
    image_bit_data ibd = { 0 };
    ibd.lpBits = NULL;

    /* Initialize the 'png_image' structure. */
    memset(&image, 0, (sizeof image));
    image.version = PNG_IMAGE_VERSION;
    
    /* The first argument is the file to read: */
    if (png_image_begin_read_from_file(&image, filename) != 0)
    {
        png_bytep buffer;

        image.format = PNG_FORMAT_BGRA;

        buffer = malloc(PNG_IMAGE_SIZE(image));
        ibd.lpBits = buffer;

        if (buffer != NULL)
            png_image_finish_read(&image, NULL, buffer, 0, NULL);
    }
    
    ibd.width = image.width;
    ibd.height = image.height;
    
    png_image_free(&image);

    return ibd;
}

void free_image_bit_data(image_bit_data* ibd) {
    free(ibd->lpBits);
    ibd->lpBits = NULL;
    ibd->width = 0;
    ibd->height = 0;
}