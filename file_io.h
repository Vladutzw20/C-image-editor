#ifndef FILE_IO_H
#define FILE_IO_H

#include "structuri.h"

void free_image_resources(image_t *img);
void load_image(const char *filename, image_t *img);
void save_image(const char *filename, image_t *img, bool is_ascii);

#endif