#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "structuri.h"
#include "cmd.h"
#include "file_io.h"

void select_coords(char *cmd, image_t *img) {
    if (!img->is_loaded) {
        printf("No image loaded\n");
        return;
    }

    int x1, y1, x2, y2;
    int parsed = sscanf(cmd, "%*s %d %d %d %d", &x1, &y1, &x2, &y2);
    
    if (parsed != 4) {
        printf("Invalid command\n");
        return;
    }

    int temp;
    if (x1 > x2) {
        temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if (y1 > y2) {
        temp = y1;
        y1 = y2;
        y2 = temp;
    }

    if (x1 < 0 || x2 > img->width || y1 < 0 || y2 > img->height || x1 == x2 || y1 == y2) {
        printf("Invalid set of coordinates\n");
        return;
    }

    img->x1 = x1;
    img->y1 = y1;
    img->x2 = x2;
    img->y2 = y2;

    printf("Selected %d %d %d %d\n", img->x1, img->y1, img->x2, img->y2);
}

void select_all(image_t *img) {
    if (!img->is_loaded) {
        printf("No image loaded\n");
        return;
    }

    img->x1 = 0;
    img->y1 = 0;
    img->x2 = img->width;
    img->y2 = img->height;

    printf("Selected ALL\n");
}

void crop_image(image_t *img) {
    if (!img->is_loaded) {
        printf("No image loaded\n");
        return;
    }

    int new_width = img->x2 - img->x1;
    int new_height = img->y2 - img->y1;

    bool is_color = (strcmp(img->magic_word, "P3") == 0 || strcmp(img->magic_word, "P6") == 0);

    if (is_color) {
        pixel_rgb **new_matrix = (pixel_rgb **)malloc(new_height * sizeof(pixel_rgb *));
        for (int i = 0; i < new_height; i++) {
            new_matrix[i] = (pixel_rgb *)malloc(new_width * sizeof(pixel_rgb));
            for (int j = 0; j < new_width; j++) {
                new_matrix[i][j] = img->color_matrix[img->y1 + i][img->x1 + j];
            }
        }

        for (int i = 0; i < img->height; i++) {
            free(img->color_matrix[i]);
        }
        free(img->color_matrix);

        img->color_matrix = new_matrix;
    } else {
        unsigned char **new_matrix = (unsigned char **)malloc(new_height * sizeof(unsigned char *));
        for (int i = 0; i < new_height; i++) {
            new_matrix[i] = (unsigned char *)malloc(new_width * sizeof(unsigned char));
            for (int j = 0; j < new_width; j++) {
                new_matrix[i][j] = img->gray_matrix[img->y1 + i][img->x1 + j];
            }
        }

        for (int i = 0; i < img->height; i++) {
            free(img->gray_matrix[i]);
        }
        free(img->gray_matrix);

        img->gray_matrix = new_matrix;
    }

    img->width = new_width;
    img->height = new_height;
    img->x1 = 0;
    img->y1 = 0;
    img->x2 = new_width;
    img->y2 = new_height;

    printf("Image cropped\n");
}

void run(char *cmd, image_t *img) {
    char command_name[32];
    
    int parsed = sscanf(cmd, "%s", command_name);
    
    if (parsed < 1) {
        return; 
    }
    
    if (strcmp(command_name, "LOAD") == 0) {
        char arg1[256];
        if (sscanf(cmd, "%*s %s", arg1) == 1) {
            load_image(arg1, img);
        } else {
            printf("Invalid command\n");
        }
    } 
    else if (strcmp(command_name, "SAVE") == 0) {
        char arg1[256];
        char arg2[256];
        arg2[0] = '\0';
        int count = sscanf(cmd, "%*s %s %s", arg1, arg2);
        if (count >= 1) {
            bool is_ascii = (strcmp(arg2, "ascii") == 0);
            save_image(arg1, img, is_ascii);
        } else {
            printf("Invalid command\n");
        }
    }
    else if (strcmp(command_name, "SELECT") == 0) {
        char arg1[256];
        int count = sscanf(cmd, "%*s %s", arg1);
        if (count >= 1 && strcmp(arg1, "ALL") == 0) {
            select_all(img);
        } else {
            select_coords(cmd, img);
        }
    }
    else if (strcmp(command_name, "CROP") == 0) {
        crop_image(img);
    }
    else {
        printf("Invalid command\n");
    }
}