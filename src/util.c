#include "util.h"
#include <stdio.h>
#include <stdlib.h>

void panic(const char *msg) {
    fprintf(stderr, "PANIC: %s\n", msg);
    abort();
}
