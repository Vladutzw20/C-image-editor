# Image Editor in C

## Project Overview
This project is a command-line image editor implemented in C. It is designed to load, manipulate, and save image files conforming to the Netpbm standard. Currently, the program supports Grayscale (PGM) and RGB Color (PPM) images, handling both their ASCII (P2, P3) and Binary (P5, P6) variants. 

The main focus of the current implementation is robust dynamic memory management and clean file parsing, ensuring that no memory leaks occur during the execution or upon exiting the program.

## Data Structures

* `pixel_rgb`: A structure that holds three `unsigned char` values (r, g, b), representing the Red, Green, and Blue channels of a single color pixel.
* `image_t`: The core structure representing the state of the currently loaded image. It contains:
  - A boolean flag indicating if an image is actively loaded in memory.
  - Metadata such as the magic word, width, height, and maximum color value.
  - Coordinates for the current selection (x1, y1, x2, y2).
  - Two distinct dynamically allocated 2D arrays (matrices): `gray_matrix` for Grayscale images and `color_matrix` for RGB images. Only one is allocated at a time based on the loaded file type.

## Core Modules and Functions

### image_editor.c (Main Entry)
* `main()`: The entry point of the program. It initializes the empty `image_t` structure, reads commands continuously from standard input, and directly handles the `EXIT` command to ensure resources are gracefully freed before termination.

### cmd.c / cmd.h (Command Router)
* `run(char *cmd, image_t *img)`: Acts as the command dispatcher. It parses the raw string input from the user, identifies the specific command (e.g., LOAD, SAVE), and calls the corresponding logic functions. Invalid commands are intercepted here.

### file_io.c / file_io.h (Input/Output Management)
* `load_image(const char *filename, image_t *img)`: Opens a specified file, parses the Netpbm header, safely skips any comments, and dynamically allocates the required memory matrix. It reads both ASCII and Binary pixel data into the memory. If a previous image was loaded, it frees it first.
* `save_image(const char *filename, image_t *img, bool is_ascii)`: Writes the current image matrix from RAM back to the disk. It can convert the output format between ASCII and Binary based on the user's explicit command parameter, regardless of the original file's format.
* `free_image_resources(image_t *img)`: Iterates through the dynamically allocated 2D arrays (pixels) and frees them row by row, followed by the main array pointer. It guarantees that the program stays memory-efficient and passes Valgrind checks.
* `skip_comments(FILE *file)`: A helper function that reads through the file stream and ignores any lines starting with the `#` character, which is essential for parsing standard Netpbm headers.