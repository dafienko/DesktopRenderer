#define _CRT_SECURE_NO_WARNINGS

#include "assetLoader.h"
#include "lpng1637/png.h"
#include <stdio.h>
#include <stdlib.h>
#include "errors.h"

#define uch unsigned char
#define uint unsigned int


// http://zarb.org/~gc/html/libpng.html
image_bit_data read_png_file(const char* filename) {
    image_bit_data ibd = { 0 };
    
    int x, y;

    int width, height;
    png_byte color_type;
    png_byte bit_depth;

    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
    png_bytep* row_pointers;

    char header[8];    // 8 is the maximum size that can be checked

        /* open file and test for it being a png */
    FILE* fp = fopen(filename, "rb");
    if (!fp)
        return;

    fread(header, 1, 8, fp);

    if (png_sig_cmp(header, 0, 8))
        return;


    /* initialize stuff */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
        return;

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        return;

    if (setjmp(png_jmpbuf(png_ptr)))
        return;

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);


    /* read file */
    if (setjmp(png_jmpbuf(png_ptr)))
        return;

    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for (y = 0; y < height; y++)
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));

    png_read_image(png_ptr, row_pointers);



    png_bytep buffer = malloc(width * height * 4);

    if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
        return;

    if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA)
        return;

    for (y = 0; y < height; y++) {
        png_byte* row = row_pointers[y];
        for (x = 0; x < width; x++) {
            png_byte* ptr = &(row[x * 4]);

            png_byte r, g, b, a;
            r = ptr[0];
            g = ptr[1];
            b = ptr[2];
            a = ptr[3];

            for (int c = 0; c < 4; c++) {
                buffer[(y * width + x) * 4 + c] = ptr[c];
            }
        }
    }

    fclose(fp);
    for (y = 0; y < height; y++)
        free(row_pointers[y]);
    free(row_pointers);

    ibd.width = width;
    ibd.height = height;
    ibd.lpBits = buffer;
    ibd.colorType = color_type;

    return ibd;
}

image_bit_data read_png_file_simple(const char* filename) {
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

        image.format = PNG_FORMAT_RGBA;

        buffer = malloc(PNG_IMAGE_SIZE(image));
        ibd.lpBits = buffer;

        png_color backgroundColor = { 255u, 0, 0 };

        if (buffer != NULL)
            png_image_finish_read(&image, NULL, buffer, 0, NULL);
    }
    
    

    ibd.width = image.width;
    ibd.height = image.height;
    ibd.colorType = PNG_FORMAT_BGRA;
    
    png_image_free(&image);

    return ibd;
}

void free_image_bit_data(image_bit_data* ibd) {
    free(ibd->lpBits);
    ibd->lpBits = NULL;
    ibd->width = 0;
    ibd->height = 0;
}
