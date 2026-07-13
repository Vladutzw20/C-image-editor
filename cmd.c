#include <stdio.h>
#include <string.h>
#include "structuri.h"
#include "cmd.h"
#include "file_io.h"

void run(char *cmd, image_t *img) {
    char command_name[32];
    char arg1[256];
    
    int parsed = sscanf(cmd, "%s %s", command_name, arg1);
    
    if (parsed < 1) {
        return; 
    }
    
    if (strcmp(command_name, "LOAD") == 0) {
        if (parsed == 2) {
            load_image(arg1, img);
            debug_print_image_to_file(img, "test_output.txt");
        } else {
            printf("Invalid command\n");
        }
    } 
    else {
        printf("Invalid command\n");
    }
}