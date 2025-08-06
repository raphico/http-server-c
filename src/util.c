#include "util.h"
#include <stdio.h>
#include <stdlib.h>

void panic(const char *msg) {
    fprintf(stderr, "PANIC: %s\n", msg);
    abort();
}

void print_usage() {
    printf("Usage: ./your_program [--directory <path>]\n");
    printf("Options:\n");
    printf("  --directory <path>   Specify the directory where files will be "
           "stored\n");
}
