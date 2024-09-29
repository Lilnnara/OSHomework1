#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H

#include <unistd.h>    // For ssize_t
#include <sys/types.h> // For ssize_t

// Function declarations
size_t my_strlen(const char *s);
int my_putc(int c, int fd);
int my_file_puts(int fd, const char *s);
int my_atoi(const char *str);
void display_error(const char *message);
ssize_t read_file(int fd, char *buffer, size_t count);
int str_cmp(const char *s1, const char *s2);
int str_n_cmp(const char *s1, const char *s2, size_t n);
void *my_memcpy(void *dest, const void *src, size_t n);
void *my_memset(void *s, int c, size_t n);

#endif // MY_FUNCTIONS_H
