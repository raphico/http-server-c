#include "header.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char *canonicalize_key(const char *key);

int headers_init(headers_t *headers) {
    if (!headers) {
        return -1;
    }

    headers->capacity = MAX_HEADERS;

    headers->count = 0;

    headers->items = malloc(sizeof(header_t) * MAX_HEADERS);
    if (!headers->items) {
        return -1;
    }

    return 0;
}

void headers_add(headers_t *headers, const char *key, const char *value) {
    if (headers->count == headers->capacity) {
        headers->capacity *= 2;
        headers->items =
            realloc(headers->items, sizeof(header_t) * headers->capacity);
    }

    headers->items[headers->count] = (header_t){
        .key = canonicalize_key(key),
        .value = strdup(value),
    };

    headers->count++;
}

char *canonicalize_key(const char *key) {
    int len = strlen(key);
    char *canonical_key = malloc(len + 1);

    for (int i = 0; i < len; i++) {
        canonical_key[i] = tolower(key[i]);
    }
    canonical_key[len] = '\0';

    return canonical_key;
}

void headers_cleanup(headers_t *headers) {
    if (!headers) {
        return;
    }

    for (int i = 0, n = headers->count; i < n; i++) {
        free(headers->items[i].key);
        free(headers->items[i].value);
    }

    free(headers->items);
    headers->count = 0;
    headers->capacity = 0;
}
