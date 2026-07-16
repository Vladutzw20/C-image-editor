#ifndef CMD_H
#define CMD_H

#include "structuri.h"

void select_coords(char *cmd, image_t *img);
void select_all(image_t *img);
void crop_image(image_t *img);
void run(char *cmd, image_t *img);

#endif