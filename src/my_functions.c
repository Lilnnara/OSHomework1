#include "my_functions.h"
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>  // This defines size_t



size_t my_strlen(const char *s) {
    size_t i = 0;
    while (*s++ != '\0')
        ++i;
    return i;
}


int my_putc(int c, int fd) {
    if (write(fd, &c, 1) != 1) {
        return EOF;
    }
    return c;
}

int my_file_puts(int fd, const char *s) {
    int c;
    while((c = *s++)) {
        if (my_putc(c, fd) == EOF) {
            const char *error_msg = "Error writing to file: ";
            write(2, error_msg, my_strlen(error_msg));
            write(2, "\n", 1);
            return EOF;
        }
    }
    return 0; 
}

int my_atoi(const char *str) {
    int result = 0;

    if (*str == '\0') {
        // Empty string
        return -1;
    }

    if (*str == '-') {
        // Negative numbers are invalid for this program
        return -1;
    }

    if (*str == '+') {
        str++;
    }

    while (*str) {
        if (*str >= '0' && *str <= '9') {
            result = result * 10 + (*str - '0');
        } else {
            // Non-numeric character encountered
            return -1;
        }
        str++;
    }

    return result;
}


void display_error(const char *message) {
    write(2, message, my_strlen(message));
    write(2, "\n", 1);
}

ssize_t read_file(int fd, char *buffer, size_t count) {
    ssize_t bytes_read = read(fd, buffer, count);
    if (bytes_read == -1) {
        display_error("Error reading file");
    }
    return bytes_read;
}


int str_cmp(const char *s1, const char *s2) {
  return str_n_cmp(s1, s2, (size_t)-1);
}

int str_n_cmp(const char *s1, const char *s2, size_t n) {
  if (n == 0) return 0;

  while (n-- && *s1 != '\0' && *s1 == *s2) {
    s1++;
    s2++;
  }
  return ((const unsigned char)*s1) - ((const unsigned char)*s2);
}

void *my_memcpy(void *dest, const void *src, size_t n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}


void *my_memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}
