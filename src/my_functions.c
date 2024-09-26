#include "my_functions.h"
#include <stdio.h>
#include <unistd.h>


size_t str_len(const char *s)
{
    size_t i = 0;
    while (*s++ != '\0')
        ++i;
    return i;
}

int my_putc(int c, int fd) {
    // Write the character to the file descriptor
    if (write(fd, &c, 1) != 1) {
        // If write fails, return EOF
        return EOF;
    }
    return c; // Return the character written on success
}


int my_file_puts(int fd, const char *s)
{
    int c;
    while((c = *s++)) {
        if (my_putc(c, fd) == EOF) { 
            // Check if putc failed
            // Print an error message using strerror

            // Create an error message
            const char *error_msg = "Error writing to file: ";

            // Write the error message to stderr
            write(2, error_msg, str_len(error_msg));

            write(2, "\n", 1); // Write a new line for format

            return EOF; // Return EOF to indicate failure
        }
    }
    // return 0 to inidcate sucess
    return 0; 
}
