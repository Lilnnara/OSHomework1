#include <unistd.h>    // read(), write(), close()
#include <fcntl.h>     // open()
#include <stdlib.h>    // malloc(), free(), exit()
#include "my_functions.h" 
#include <sys/types.h> //  ssize_t

int tail_file(int fd, int num_lines);

int main(int argc, char *argv[]) {
    // Variables to store options and filename
    int num_lines = 10;    // Default number of lines to display
    char *filename = NULL; // Pointer to store the filename if provided

    // Index variable for iterating through arguments
    int i = 1; // Start from 1 to skip the program name

    // Loop through the command-line arguments
    while (i < argc) {
        // Check if the current argument is '-n'
        if (my_strlen(argv[i]) == 2 && argv[i][0] == '-' && argv[i][1] == 'n') {
            // Ensure that '-n' is followed by a number
            if (i + 1 < argc) {
                // Convert the next argument to an integer
                num_lines = my_atoi(argv[i + 1]);
                // Validate that num_lines is positive
                if (num_lines <= 0) {
                    my_file_puts(STDERR_FILENO, "Error: Invalid number after '-n' option.\n");
                    return 1; // Exit with an error code
                }
                i += 2; // Move past the '-n' and the number
            } else {
                // Error: '-n' provided without a following number
                my_file_puts(STDERR_FILENO, "Error: Missing number after '-n' option.\n");
                return 1; // Exit with an error code
            }
        }
        // If the argument does not start with '-', treat it as a filename
        else if (argv[i][0] != '-') {
            if (filename == NULL) {
                filename = argv[i]; // Store the filename
                i++; // Move to the next argument
            } else {
                // Error: Multiple filenames provided
                my_file_puts(STDERR_FILENO, "Error: Multiple filenames are not supported.\n");
                return 1; // Exit with an error code
            }
        }
        // Handle unrecognized options
        else {
            my_file_puts(STDERR_FILENO, "Error: Unrecognized option.\n");
            return 1; // Exit with an error code
        }
    }

    // Variables to store file descriptor
    int fd; // File descriptor

    // Open the file or set up to read from stdin
    if (filename != NULL) {
        // A filename was provided; attempt to open it
        fd = open(filename, O_RDONLY);
        if (fd == -1) {
            // Error occurred while opening the file
            my_file_puts(STDERR_FILENO, "Error: Cannot open file.\n");
            return 1; // Exit with an error code
        }
    } else {
        // No filename provided; read from stdin
        fd = STDIN_FILENO; // Standard input file descriptor
    }

    // Call the tail_file function
    if (tail_file(fd, num_lines) != 0) {
        // An error occurred
        if (filename != NULL) {
            close(fd);
        }
        return 1;
    }

    // Close the file if it was opened
    if (filename != NULL) {
        close(fd);
    }

    return 0;  // Return 0 to indicate successful execution
}


int tail_file(int fd, int num_lines) {
    // Allocate memory for an array of pointers to hold the lines
    char **lines = malloc(num_lines * sizeof(char *));
    if (lines == NULL) {
        my_file_puts(STDERR_FILENO, "Error: Memory allocation failed.\n");
        return 1;  // Return error code
    }

    // Initialize the lines array
    for (int i = 0; i < num_lines; i++) {
        lines[i] = NULL;
    }

    // Variables for reading and processing input
    char buffer[1024];  // Buffer for reading input
    ssize_t bytes_read; // Number of bytes read
    int total_lines_read = 0;      // Total number of lines read
    int current_line_index = 0;    // Index in the circular buffer

    // Variables for building lines
    char *line_buffer = NULL;      // Buffer for the current line
    size_t line_size = 0;          // Current size of the line
    size_t line_capacity = 0;      // Capacity of the line buffer

// Reading loop
while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
    for (ssize_t i = 0; i < bytes_read; i++) {
        char c = buffer[i];

        // Expand line_buffer if necessary
        if (line_size + 1 > line_capacity) {
            size_t new_capacity = line_capacity + 128;  // Increase capacity
            char *new_line_buffer = realloc(line_buffer, new_capacity);
            if (new_line_buffer == NULL) {
                my_file_puts(STDERR_FILENO, "Error: Memory allocation failed.\n");
                // Free allocated resources
                free(line_buffer);
                for (int j = 0; j < num_lines; j++) {
                    free(lines[j]);
                }
                free(lines);
                return 1;
            }
            line_buffer = new_line_buffer;
            line_capacity = new_capacity;
        }

        // Append the character to line_buffer
        line_buffer[line_size++] = c;

        // Check for end of line
        if (c == '\n') {
            // Null-terminate the line
            line_buffer[line_size] = '\0';

            // Store the line in the circular buffer
            // Free the old line if it exists
            if (lines[current_line_index] != NULL) {
                free(lines[current_line_index]);
            }
            lines[current_line_index] = line_buffer;

            // Update the circular buffer index
            current_line_index = (current_line_index + 1) % num_lines;

            // Reset line_buffer variables for the next line
            line_buffer = NULL;
            line_size = 0;
            line_capacity = 0;

            // Increment the total lines read
            total_lines_read++;
        }
    }
}

// Handle any read errors
if (bytes_read == -1) {
    my_file_puts(STDERR_FILENO, "Error: Failed to read from input.\n");
    // Free allocated resources
    free(line_buffer);
    for (int i = 0; i < num_lines; i++) {
        free(lines[i]);
    }
    free(lines);
    return 1;
}

// After the loop, check if there's any remaining data in line_buffer
if (line_size > 0) {
    // Null-terminate the last line
    line_buffer[line_size] = '\0';

    // Store the last line in the circular buffer
    if (lines[current_line_index] != NULL) {
        free(lines[current_line_index]);
    }
    lines[current_line_index] = line_buffer;

    // Update the index and total lines read
    current_line_index = (current_line_index + 1) % num_lines;
    total_lines_read++;
} else {
    // Free line_buffer if it was allocated
    free(line_buffer);
}

// Determine how many lines to output
int lines_to_output = (total_lines_read < num_lines) ? total_lines_read : num_lines;

// Calculate the starting index
int start_index;
if (total_lines_read < num_lines) {
    // If we read fewer lines than num_lines, start from 0
    start_index = 0;
} else {
    // Start from current_line_index
    start_index = current_line_index % num_lines;
}

// Output the lines
for (int i = 0; i < lines_to_output; i++) {
    int index = (start_index + i) % num_lines;
    if (lines[index] != NULL) {
        my_file_puts(STDOUT_FILENO, lines[index]);
    }
}

    // Free allocated memory
    for (int i = 0; i < num_lines; i++) {
        free(lines[i]);
    }
    free(lines);

    return 0;  // Return success code
}

