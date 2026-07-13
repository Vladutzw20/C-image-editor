#ifndef STRUCTURI_H
#define STRUCTURI_H

#include <stdbool.h>

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} pixel_rgb;

typedef struct {
    bool is_loaded;
    char magic_word[3];
    int width;
    int height;
    int max_value;
    
    int x1;
    int y1;
    int x2;
    int y2;
    
    unsigned char **gray_matrix;
    pixel_rgb **color_matrix;
} image_t;

#endif