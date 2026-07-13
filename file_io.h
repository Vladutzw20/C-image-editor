#ifndef FILE_IO_H
#define FILE_IO_H

#include "structuri.h"

void free_image_resources(image_t *img);
void load_image(const char *filename, image_t *img);
void debug_print_image_to_file(image_t *img, const char *filename);

#endif