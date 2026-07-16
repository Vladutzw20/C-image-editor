#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "structuri.h"
#include "cmd.h"
#include "file_io.h"

// Funcția clamp cerută în enunț
int clamp(int val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return val;
}

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

void apply_kernel(image_t *img, double kernel[3][3]) {
    int w = img->width;
    int h = img->height;
    
    pixel_rgb **temp = malloc(h * sizeof(pixel_rgb *));
    for (int i = 0; i < h; i++) {
        temp[i] = malloc(w * sizeof(pixel_rgb));
        for (int j = 0; j < w; j++) {
            temp[i][j] = img->color_matrix[i][j];
        }
    }

    for (int i = img->y1; i < img->y2; i++) {
        for (int j = img->x1; j < img->x2; j++) {
            if (i == 0 || j == 0 || i == h - 1 || j == w - 1) continue;

            double sum_r = 0, sum_g = 0, sum_b = 0;
            for (int ki = 0; ki < 3; ki++) {
                for (int kj = 0; kj < 3; kj++) {
                    pixel_rgb p = temp[i - 1 + ki][j - 1 + kj];
                    double kv = kernel[ki][kj];
                    sum_r += p.r * kv;
                    sum_g += p.g * kv;
                    sum_b += p.b * kv;
                }
            }
        
            img->color_matrix[i][j].r = (unsigned char)clamp((int)round(sum_r));
            img->color_matrix[i][j].g = (unsigned char)clamp((int)round(sum_g));
            img->color_matrix[i][j].b = (unsigned char)clamp((int)round(sum_b));
        }
    }

    for (int i = 0; i < h; i++) free(temp[i]);
    free(temp);
}

void apply_command(char *param, image_t *img) {
    if (!img->is_loaded) {
        printf("No image loaded\n");
        return;
    }

    bool is_color = (strcmp(img->magic_word, "P3") == 0 || strcmp(img->magic_word, "P6") == 0);
    
    double EDGE[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};
    double SHARPEN[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};
    double BLUR[3][3] = {{1.0/9, 1.0/9, 1.0/9}, {1.0/9, 1.0/9, 1.0/9}, {1.0/9, 1.0/9, 1.0/9}};
    double GAUSS[3][3] = {{1.0/16, 2.0/16, 1.0/16}, {2.0/16, 4.0/16, 2.0/16}, {1.0/16, 2.0/16, 1.0/16}};
    
    double (*kernel)[3] = NULL;
    if (strcmp(param, "EDGE") == 0) kernel = EDGE;
    else if (strcmp(param, "SHARPEN") == 0) kernel = SHARPEN;
    else if (strcmp(param, "BLUR") == 0) kernel = BLUR;
    else if (strcmp(param, "GAUSSIAN_BLUR") == 0) kernel = GAUSS;
    else {
        printf("APPLY parameter invalid\n");
        return;
    }

    if (!is_color) {
        printf("Easy, Charlie Chaplin\n");
        return;
    }

    apply_kernel(img, kernel);
    printf("APPLY %s done\n", param);
}

void rotate_90(image_t *img) {
    bool is_color = (strcmp(img->magic_word, "P3") == 0 || strcmp(img->magic_word, "P6") == 0);
    bool is_full = (img->x1 == 0 && img->y1 == 0 && img->x2 == img->width && img->y2 == img->height);
    int sel_w = img->x2 - img->x1;
    int sel_h = img->y2 - img->y1;

    if (is_full) {
        if (is_color) {
            pixel_rgb **new_mat = malloc(sel_w * sizeof(pixel_rgb *));
            for (int i = 0; i < sel_w; i++) {
                new_mat[i] = malloc(sel_h * sizeof(pixel_rgb));
                for (int j = 0; j < sel_h; j++) {
                    new_mat[i][j] = img->color_matrix[sel_h - 1 - j][i];
                }
            }
            for (int i = 0; i < sel_h; i++) free(img->color_matrix[i]);
            free(img->color_matrix);
            img->color_matrix = new_mat;
        } else {
            unsigned char **new_mat = malloc(sel_w * sizeof(unsigned char *));
            for (int i = 0; i < sel_w; i++) {
                new_mat[i] = malloc(sel_h * sizeof(unsigned char));
                for (int j = 0; j < sel_h; j++) {
                    new_mat[i][j] = img->gray_matrix[sel_h - 1 - j][i];
                }
            }
            for (int i = 0; i < sel_h; i++) free(img->gray_matrix[i]);
            free(img->gray_matrix);
            img->gray_matrix = new_mat;
        }
        img->width = sel_h;
        img->height = sel_w;
        img->x1 = 0;
        img->y1 = 0;
        img->x2 = img->width;
        img->y2 = img->height;
    } else {
        if (is_color) {
            pixel_rgb **temp = malloc(sel_h * sizeof(pixel_rgb *));
            for (int i = 0; i < sel_h; i++) {
                temp[i] = malloc(sel_w * sizeof(pixel_rgb));
                for (int j = 0; j < sel_w; j++) {
                    temp[i][j] = img->color_matrix[img->y1 + sel_h - 1 - j][img->x1 + i];
                }
            }
            for (int i = 0; i < sel_h; i++) {
                for (int j = 0; j < sel_w; j++) {
                    img->color_matrix[img->y1 + i][img->x1 + j] = temp[i][j];
                }
            }
            for (int i = 0; i < sel_h; i++) free(temp[i]);
            free(temp);
        } else {
            unsigned char **temp = malloc(sel_h * sizeof(unsigned char *));
            for (int i = 0; i < sel_h; i++) {
                temp[i] = malloc(sel_w * sizeof(unsigned char));
                for (int j = 0; j < sel_w; j++) {
                    temp[i][j] = img->gray_matrix[img->y1 + sel_h - 1 - j][img->x1 + i];
                }
            }
            for (int i = 0; i < sel_h; i++) {
                for (int j = 0; j < sel_w; j++) {
                    img->gray_matrix[img->y1 + i][img->x1 + j] = temp[i][j];
                }
            }
            for (int i = 0; i < sel_h; i++) free(temp[i]);
            free(temp);
        }
    }
}

void rotate_command(int angle, image_t *img) {
    if (!img->is_loaded) {
        printf("No image loaded\n");
        return;
    }

    if (angle != 90 && angle != 180 && angle != 270 && angle != 360 &&
        angle != -90 && angle != -180 && angle != -270 && angle != -360 && angle != 0) {
        printf("Unsupported rotation angle\n");
        return;
    }

    bool is_full = (img->x1 == 0 && img->y1 == 0 && img->x2 == img->width && img->y2 == img->height);
    int sel_w = img->x2 - img->x1;
    int sel_h = img->y2 - img->y1;

    if (!is_full && sel_w != sel_h) {
        printf("The selection must be square\n");
        return;
    }

    int steps = (angle / 90) % 4;
    if (steps < 0) steps += 4;

    for (int i = 0; i < steps; i++) {
        rotate_90(img);
    }

    printf("Rotated %d\n", angle);
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
    else if (strcmp(command_name, "APPLY") == 0) {
        char arg1[256];
        arg1[0] = '\0';
        sscanf(cmd, "%*s %s", arg1);
        if (arg1[0] == '\0') {
            printf("APPLY parameter invalid\n");
        } else {
            apply_command(arg1, img);
        }
    }
    else if (strcmp(command_name, "ROTATE") == 0) {
        int angle;
        if (sscanf(cmd, "%*s %d", &angle) == 1) {
            rotate_command(angle, img);
        } else {
            printf("Invalid command\n");
        }
    }
    else {
        printf("Invalid command\n");
    }
}