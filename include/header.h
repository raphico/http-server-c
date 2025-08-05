#pragma once

#include <stddef.h>

enum { MAX_HEADERS = 16 };

typedef struct {
    char *key;
    char *value;
} header_t;

typedef struct {
    header_t *items;
    size_t count;
    size_t capacity;
} headers_t;

int headers_init(headers_t *headers);
void headers_add(headers_t *headers, const char *key, const char *value);
char *headers_get(headers_t *headers, const char *key);
void headers_cleanup(headers_t *headers);
