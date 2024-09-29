#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include "my_functions.h"

#define LINE_SIZE 32 // Each line is exactly 32 bytes
#define PREFIX_SIZE 6
#define LOCATION_SIZE 25

// Function prototypes
void display_usage();
int is_valid_number(const char *str);
off_t get_file_size(int fd);
int binary_search(char *data, size_t num_records, const char *target_prefix, char *result_location);
int linear_search(char *data, size_t data_size, const char *target_prefix, char *result_location);
void trim_trailing_spaces(char *str);

int main(int argc, char *argv[]) {
    int fd = -1; // File descriptor
    int use_stdin = 1; // Flag to check if we are using stdin
    char *filename = NULL;
    char *number = NULL;
    char result_location[LOCATION_SIZE + 1]; // +1 for null terminator
    char target_prefix[PREFIX_SIZE + 1]; // +1 for null terminator

    // Argument Parsing
    if (argc == 2) {
        number = argv[1];
    } else if (argc >= 3) {
        number = argv[1];
        filename = argv[2];
        use_stdin = 0;
    } else {
        display_usage();
        return 1;
    }

    // Validate the number
    if (!is_valid_number(number)) {
        display_error("Invalid number format. Please provide a 10-digit number.");
        return 1;
    }

    // Extract the first 6 digits to create the target prefix
    my_memcpy(target_prefix, number, PREFIX_SIZE);
    target_prefix[PREFIX_SIZE] = '\0';

    // Open the file
    if (use_stdin) {
        fd = STDIN_FILENO;
    } else {
        fd = open(filename, O_RDONLY);
        if (fd == -1) {
            display_error("Error opening file");
            return 1;
        }
    }

    // Check if lseek works on fd
    off_t file_size = lseek(fd, 0, SEEK_CUR);
    int lseekable = 1;
    if (file_size == -1 && errno == ESPIPE) {
        // File descriptor is not seekable
        lseekable = 0;
    } else {
        // Reset file offset to beginning
        lseek(fd, 0, SEEK_SET);
    }

    // Proceed based on whether fd is seekable
    int result = -1;
    if (lseekable) {
        // Seekable file descriptor, use mmap and binary search
        file_size = get_file_size(fd);
        if (file_size == -1) {
            display_error("Error getting file size");
            if (!use_stdin) close(fd);
            return 1;
        }

        // Map the file into memory
        char *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (data == MAP_FAILED) {
            display_error("Error mapping file into memory");
            if (!use_stdin) close(fd);
            return 1;
        }

        size_t num_records = file_size / LINE_SIZE;

        result = binary_search(data, num_records, target_prefix, result_location);

        // Unmap the memory
        if (munmap(data, file_size) == -1) {
            display_error("Error unmapping memory");
        }
    } else {
        // Non-seekable file descriptor, read into buffer and use linear search
        // We'll read the data into a dynamically allocated buffer
        size_t buffer_size = 1024 * LINE_SIZE; // Initial buffer size
        size_t data_size = 0;
        char *data = malloc(buffer_size);
        if (!data) {
            display_error("Memory allocation error");
            if (!use_stdin) close(fd);
            return 1;
        }

        ssize_t bytes_read;
        while ((bytes_read = read(fd, data + data_size, buffer_size - data_size)) > 0) {
            data_size += bytes_read;
            if (data_size == buffer_size) {
                // Need to increase buffer size
                buffer_size *= 2;
                char *temp = realloc(data, buffer_size);
                if (!temp) {
                    display_error("Memory reallocation error");
                    free(data);
                    if (!use_stdin) close(fd);
                    return 1;
                }
                data = temp;
            }
        }
        if (bytes_read == -1) {
            display_error("Error reading file");
            free(data);
            if (!use_stdin) close(fd);
            return 1;
        }

        result = linear_search(data, data_size, target_prefix, result_location);

        free(data);
    }

    if (!use_stdin) close(fd);

    if (result == 0) {
        // Trim trailing spaces
        trim_trailing_spaces(result_location);

        // Found the location
        my_file_puts(STDOUT_FILENO, result_location);
        my_putc('\n', STDOUT_FILENO);
        return 0;
    } else {
        // Not found
        display_error("Prefix not found");
        return 1;
    }
}
void display_usage() {
    display_error("Usage: findlocation <10-digit-number> [filename]");
}

int is_valid_number(const char *str) {
    if (my_strlen(str) != 10) {
        return 0;
    }
    for (size_t i = 0; i < 10; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return 0;
        }
    }
    return 1;
}


off_t get_file_size(int fd) {
    off_t current_pos = lseek(fd, 0, SEEK_CUR);
    if (current_pos == -1) {
        return -1;
    }
    off_t size = lseek(fd, 0, SEEK_END);
    if (size == -1) {
        return -1;
    }
    // Restore file offset
    if (lseek(fd, current_pos, SEEK_SET) == -1) {
        return -1;
    }
    return size;
}

int binary_search(char *data, size_t num_records, const char *target_prefix, char *result_location) {
    size_t left = 0;
    size_t right = num_records - 1;
    char prefix_buffer[PREFIX_SIZE + 1]; // +1 for null terminator

    while (left <= right) {
        size_t mid = left + (right - left) / 2;
        char *record = data + (mid * LINE_SIZE);

        // Extract the prefix
        my_memcpy(prefix_buffer, record, PREFIX_SIZE);
        prefix_buffer[PREFIX_SIZE] = '\0';

        int cmp_result = str_n_cmp(prefix_buffer, target_prefix, PREFIX_SIZE);
        if (cmp_result == 0) {
            // Extract the location
            my_memcpy(result_location, record + PREFIX_SIZE, LOCATION_SIZE);
            result_location[LOCATION_SIZE] = '\0';
            return 0; // Found
        } else if (cmp_result < 0) {
            left = mid + 1;
        } else {
            if (mid == 0) break; // Prevent underflow
            right = mid - 1;
        }
    }
    return -1; // Not found
}


int linear_search(char *data, size_t data_size, const char *target_prefix, char *result_location) {
    size_t num_records = data_size / LINE_SIZE;
    char prefix_buffer[PREFIX_SIZE + 1]; // +1 for null terminator

    for (size_t i = 0; i < num_records; i++) {
        char *record = data + (i * LINE_SIZE);

        // Extract the prefix
        my_memcpy(prefix_buffer, record, PREFIX_SIZE);
        prefix_buffer[PREFIX_SIZE] = '\0';

        int cmp_result = str_n_cmp(prefix_buffer, target_prefix, PREFIX_SIZE);
        if (cmp_result == 0) {
            // Extract the location
            my_memcpy(result_location, record + PREFIX_SIZE, LOCATION_SIZE);
            result_location[LOCATION_SIZE] = '\0';
            return 0; // Found
        }
    }
    return -1; // Not found
}

void trim_trailing_spaces(char *str) {
    int index = my_strlen(str) - 1;
    while (index >= 0 && (str[index] == ' ' || str[index] == '\n')) {
        str[index] = '\0';
        index--;
    }
}

