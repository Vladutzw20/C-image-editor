#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structuri.h"
#include "file_io.h"

void skip_comments(FILE *file) {
    char ch;
    fscanf(file, " "); 
    ch = fgetc(file);
    while (ch == '#') {
        while (ch != '\n' && ch != EOF) {
            ch = fgetc(file);
        }
        fscanf(file, " ");
        ch = fgetc(file);
    }
    ungetc(ch, file); 
}

void free_image_resources(image_t *img) {
    if (!img->is_loaded) return;

    if (strcmp(img->magic_word, "P2") == 0 || strcmp(img->magic_word, "P5") == 0) {
        if (img->gray_matrix != NULL) {
            for (int i = 0; i < img->height; i++) free(img->gray_matrix[i]);
            free(img->gray_matrix);
            img->gray_matrix = NULL;
        }
    } 
    else if (strcmp(img->magic_word, "P3") == 0 || strcmp(img->magic_word, "P6") == 0) {
        if (img->color_matrix != NULL) {
            for (int i = 0; i < img->height; i++) free(img->color_matrix[i]);
            free(img->color_matrix);
            img->color_matrix = NULL;
        }
    }
    
    img->is_loaded = false;
}

void load_image(const char *filename, image_t *img) {
    FILE *file = fopen(filename, "rb");
    
    if (!file) {
        printf("Failed to load %s\n", filename);
        free_image_resources(img); 
        return;
    }
    
    free_image_resources(img);

    fscanf(file, "%2s", img->magic_word);
    
    skip_comments(file);
    fscanf(file, "%d %d", &img->width, &img->height);
    
    skip_comments(file);
    fscanf(file, "%d", &img->max_value);
    
    fgetc(file);

    if (strcmp(img->magic_word, "P2") == 0 || strcmp(img->magic_word, "P5") == 0) {
        img->gray_matrix = (unsigned char **)malloc(img->height * sizeof(unsigned char *));
        for (int i = 0; i < img->height; i++) {
            img->gray_matrix[i] = (unsigned char *)malloc(img->width * sizeof(unsigned char));
            
            for (int j = 0; j < img->width; j++) {
                if (strcmp(img->magic_word, "P2") == 0) {
                    fscanf(file, "%hhu", &img->gray_matrix[i][j]);
                } else {
                    fread(&img->gray_matrix[i][j], sizeof(unsigned char), 1, file);
                }
            }
        }
    } 
    else if (strcmp(img->magic_word, "P3") == 0 || strcmp(img->magic_word, "P6") == 0) {
        img->color_matrix = (pixel_rgb **)malloc(img->height * sizeof(pixel_rgb *));
        for (int i = 0; i < img->height; i++) {
            img->color_matrix[i] = (pixel_rgb *)malloc(img->width * sizeof(pixel_rgb));
            
            for (int j = 0; j < img->width; j++) {
                if (strcmp(img->magic_word, "P3") == 0) {
                    fscanf(file, "%hhu %hhu %hhu", 
                           &img->color_matrix[i][j].r, 
                           &img->color_matrix[i][j].g, 
                           &img->color_matrix[i][j].b);
                } else {
                    fread(&img->color_matrix[i][j], sizeof(pixel_rgb), 1, file);
                }
            }
        }
    }

    fclose(file);

    img->is_loaded = true;
    img->x1 = 0;
    img->y1 = 0;
    img->x2 = img->width;
    img->y2 = img->height;
    
    printf("Loaded %s\n", filename);
}

void save_image(const char *filename, image_t *img, bool is_ascii) {
    if (!img->is_loaded) {
        printf("No image loaded\n");
        return;
    }

    FILE *file = fopen(filename, "wb");
    if (!file) {
        return;
    }

    bool is_color = (strcmp(img->magic_word, "P3") == 0 || strcmp(img->magic_word, "P6") == 0);

    if (is_color) {
        fprintf(file, "%s\n", is_ascii ? "P3" : "P6");
    } else {
        fprintf(file, "%s\n", is_ascii ? "P2" : "P5");
    }

    fprintf(file, "%d %d\n", img->width, img->height);
    fprintf(file, "%d\n", img->max_value);

    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            if (is_color) {
                if (is_ascii) {
                    fprintf(file, "%d %d %d ", 
                            img->color_matrix[i][j].r, 
                            img->color_matrix[i][j].g, 
                            img->color_matrix[i][j].b);
                } else {
                    fwrite(&img->color_matrix[i][j], sizeof(pixel_rgb), 1, file);
                }
            } else {
                if (is_ascii) {
                    fprintf(file, "%d ", img->gray_matrix[i][j]);
                } else {
                    fwrite(&img->gray_matrix[i][j], sizeof(unsigned char), 1, file);
                }
            }
        }
        if (is_ascii) {
            fprintf(file, "\n");
        }
    }

    fclose(file);
    printf("Saved %s\n", filename);
}