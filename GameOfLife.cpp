#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PNG_SETJMP_NOT_SUPPORTED
#include <png.h>

//Conway's Game of Life
//1. 죽은 칸에 접한 8칸 중 정확히 3칸에 세포가 살아 있다면 해당 칸의 세포는 그 다음 세대에 살아난다.
//2. 살아있는 칸과 접한 8칸 중 2칸 혹은 3칸에 세포가 살아 있다면 해당 칸의 세포는 살아있는 상태를 유지한다.
//3. 그 이외의 경우 해당 칸의 세포는 다음 세대에 고립돼 죽거나 혹은 주위가 너무 복잡해져서 죽는다. 혹은 죽은 상태를 유지한다.

//Image size
#define WIDTH 256
#define HEIGHT 256
#define COLOR_DEPTH 8
//Background RGB : purple
#define BACKGROUND_COLOR_R 150
#define BACKGROUND_COLOR_G 95
#define BACKGROUND_COLOR_B 212
//Living cell RGB : green
#define LIVING_CELL_COLOR_R 139
#define LIVING_CELL_COLOR_G 212
#define LIVING_CELL_COLOR_B 80

struct Pixel {
    png_byte r, g, b, a;
};

// Function to free memory
void free_image_data(struct Pixel* row_pointers[HEIGHT]) {
    for (int row = 0; row < HEIGHT; row++) {
        free(row_pointers[row]);
    }
}

// Function to get the next state of a cell in the Game of Life
int get_next_state(int current_state_r, int live_neighbors) {
    // Determines current state by comparing R value with Living_Cell_Color_R
    if (current_state_r == LIVING_CELL_COLOR_R) {
        return (live_neighbors == 2 || live_neighbors == 3) ? 1 : 0; // Game of Life rule 2
    }
    else {
        return (live_neighbors == 3) ? 1 : 0;  // Game of Life rule 1
    }
}

// Function to count the number of live neighbors for a cell
int count_live_neighbors(struct Pixel* grid[HEIGHT], int x, int y) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int neighbor_x = x + i;
            int neighbor_y = y + j;

            // Check bounds
            if (neighbor_x >= 0 && neighbor_x < WIDTH && neighbor_y >= 0 && neighbor_y < HEIGHT) {
                count += grid[neighbor_y][neighbor_x].r == LIVING_CELL_COLOR_R ? 1 : 0;
            }
        }
    }
    count -= grid[y][x].r == LIVING_CELL_COLOR_R ? 1 : 0; // Exclude the center cell itself
    return count;
}

int main(int argc, char* argv[]) {
    // Error while taking filename
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <output_filename> <current_output_filename2> <generation num>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    // Open PNG file for writing
    FILE* f1 = fopen(argv[1], "wb"); // Takes filename as a command line argument
    if (!f1) {
        fprintf(stderr, "Could not open %s\n", argv[1]);
        return 1;
    }
    FILE* f2 = fopen(argv[2], "wb"); // Takes filename as a command line argument
    if (!f2) {
        fprintf(stderr, "Could not open %s\n", argv[2]);
        return 1;
    }

    /* Initialize png data structures */
    png_structp png_ptr1, png_ptr2;
    png_infop info_ptr1, info_ptr2;

    png_ptr1 = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_ptr2 = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr1 || !png_ptr2) {
        fprintf(stderr, "could not initialize png struct\n");
        return 1;
    }

    info_ptr1 = png_create_info_struct(png_ptr1);
    info_ptr2 = png_create_info_struct(png_ptr2);
    if (!info_ptr1 || !info_ptr2) {
        png_destroy_write_struct(&png_ptr1, (png_infopp)NULL);
        png_destroy_write_struct(&png_ptr2, (png_infopp)NULL);
        fclose(f1);
        fclose(f2);
        return 1;
    }

    /* Begin writing PNG File */
    png_init_io(png_ptr1, f1);
    png_set_IHDR(png_ptr1, info_ptr1, WIDTH, HEIGHT, COLOR_DEPTH,
        PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr1, info_ptr1);
    png_init_io(png_ptr2, f2);
    png_set_IHDR(png_ptr2, info_ptr2, WIDTH, HEIGHT, COLOR_DEPTH,
        PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr2, info_ptr2);

    /* Allocate image data */
    struct Pixel* row_pointers[HEIGHT];
    for (int row = 0; row < HEIGHT; row++) {
        row_pointers[row] = (struct Pixel*)calloc(WIDTH, sizeof(struct Pixel));
    }

    /* Initialize grid with random initial state */
    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            row_pointers[row][col].r = (rand() % 2) ? LIVING_CELL_COLOR_R : BACKGROUND_COLOR_R;
            row_pointers[row][col].g = (row_pointers[row][col].r == LIVING_CELL_COLOR_R) ? LIVING_CELL_COLOR_G : BACKGROUND_COLOR_G;
            row_pointers[row][col].b = (row_pointers[row][col].r == LIVING_CELL_COLOR_R) ? LIVING_CELL_COLOR_B : BACKGROUND_COLOR_B;
            row_pointers[row][col].a = 255; // alpha (opacity)
        }
    }

    /* Write image data to disk for first generation (optional) */
    png_write_image(png_ptr1, (png_byte**)row_pointers);

    /* Run the Game of Life simulation for a few generations */
    for (int generation = 0; generation < atoi(argv[3]) - 1; generation++) {
        struct Pixel* new_row_pointers[HEIGHT];
        for (int row = 0; row < HEIGHT; row++) {
            new_row_pointers[row] = (struct Pixel*)calloc(WIDTH, sizeof(struct Pixel));
        }

        // Update each cell based on the rules of the Game of Life
        for (int row = 0; row < HEIGHT; row++) {
            for (int col = 0; col < WIDTH; col++) {
                int live_neighbors = count_live_neighbors(row_pointers, col, row);
                new_row_pointers[row][col].r = get_next_state(row_pointers[row][col].r, live_neighbors) ? LIVING_CELL_COLOR_R : BACKGROUND_COLOR_R;
                new_row_pointers[row][col].g = (new_row_pointers[row][col].r == LIVING_CELL_COLOR_R) ? LIVING_CELL_COLOR_G : BACKGROUND_COLOR_G; // Determines state by comparing R value
                new_row_pointers[row][col].b = (new_row_pointers[row][col].r == LIVING_CELL_COLOR_R) ? LIVING_CELL_COLOR_B : BACKGROUND_COLOR_B; // Determines state by comparing R value
                new_row_pointers[row][col].a = 255; // alpha (opacity)
            }
        }

        // Free the memory of the previous generation
        free_image_data(row_pointers);

        // Swap pointers to update the current generation
        for (int row = 0; row < HEIGHT; row++) {
            row_pointers[row] = new_row_pointers[row];
        }

        /* Write image data to disk for each generation (optional) */
        png_write_image(png_ptr2, (png_byte**)row_pointers);
    }

    // Free the final generation's memory
    free_image_data(row_pointers);

    /* Finish writing PNG file */
    png_write_end(png_ptr1, NULL);
    png_write_end(png_ptr2, NULL);

    /* Clean up PNG-related data structures */
    png_destroy_write_struct(&png_ptr1, &info_ptr1);
    png_destroy_write_struct(&png_ptr1, &info_ptr1);

    /* Close the file */
    fclose(f1);
    fclose(f2);
    f1 = NULL;
    f2 = NULL;

    return 0;
}
