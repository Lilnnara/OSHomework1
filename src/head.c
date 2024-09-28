#include "my_functions.h"
#include <fcntl.h>   // for O_RDONLY
#include <unistd.h>  // for close(), read(), write()
#include <stdlib.h> //exit


#define BUFFER_SIZE 1024 
#define DEFAULT_LINES 10 //in case we dont recieve input head will print 10 lines

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2


//helpers
/* print error in case one happens */
void print_error(const char *message) {
    write(2, message, my_strlen(message));
}
/* will print the lines indicated from the fd and the number of lines */

void print_lines(int fd, int lines_to_print) {
    char buffer[BUFFER_SIZE];
    int bytes_read, total_lines = 0, i;

    // read until there are no more lines to read and the lines printed are less than the lines to be printed
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0 && total_lines < lines_to_print) {
        // iterate over each byte in the buffer until all bytes have been read or all lines have been printed
        for (i = 0; i < bytes_read && total_lines < lines_to_print; i++) {
            //print each character in the buffer and if it returns an error a message will be printed
            if (write(STDOUT_FILENO, &buffer[i], 1) == -1) {
                //call helper to put error message
                print_error("Error writing to stdout\n");
                exit(1);
            }
            //if we find the new line thingy we increase the lines printed conter
            if (buffer[i] == '\n') {
                total_lines++;
            }
        }
    }
    //error handling in case the file is not able to be read
    if (bytes_read == -1) {
        print_error("Error reading file\n");
        exit(1);
    }
}

/* call all helpers and process arguments */


int main(int argc, char *argv[]) {
    int fd = STDIN_FILENO;  // default to stdin
    int lines_to_print = DEFAULT_LINES;
    int filename_index = -1;

    // process the arguments received with the call
    for (int i = 1; i < argc; i++) {
        //if we receive "-n" we will expect a number of lines to print instead of the default
        if (str_cmp(argv[i], "-n") == 0) {
            if (i + 1 < argc) {
                //call helper to convert the string into an int and change lines_to_print into the int
                lines_to_print = my_atoi(argv[i + 1]);
                //if we get a negative number we print the error
                if (lines_to_print < 0) {
                    print_error("Invalid number of lines\n");
                    exit(1);
                }
                i++;  // skip the number argument
            } else {
                print_error("Option -n requires an argument\n");
                exit(1);
            }
        } else {
            filename_index = i;
        }
    }

    // open the file if a filename is provided
    if (filename_index != -1) {
        fd = open(argv[filename_index], O_RDONLY);
        //error handling
        if (fd == -1) {
            print_error("Error opening file\n");
            exit(1);
        }
    }

    // pint the specified number of lines
    print_lines(fd, lines_to_print);

    // close the file if it was opened
    if (fd != STDIN_FILENO) {
        if (close(fd) == -1) {
            print_error("Error closing file\n");
            exit(1);
        }
    }

    return 0; //success!
}