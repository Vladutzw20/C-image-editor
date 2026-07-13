#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "structuri.h"
#include "cmd.h"
#include "file_io.h"

int main(void) {
    char cmd[256];
    image_t img = {0}; 
    
    while (fgets(cmd, sizeof(cmd), stdin)) {
        
        cmd[strcspn(cmd, "\n")] = '\0';
        
        if (strcmp(cmd, "EXIT") == 0) {
            if (!img.is_loaded) {
                printf("No image loaded\n");
            } else {
                free_image_resources(&img);
            }
            break; 
        }
        
        run(cmd, &img);
    }
    
    return 0;
}