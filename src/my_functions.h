#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H

#include <stdlib.h>  // size_t

size_t str_len(const char *s);

int my_putc(int c, int fd);

int my_file_puts(int fd, const char *s);

#endif 
