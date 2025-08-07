#pragma once

typedef enum {
    ENCODING_IDENTITY,
    ENCODING_GZIP,
} content_encoding_t;

enum {
    MAX_BODY_SIZE = 8192, // 8 kb
    MAX_HEADER_SIZE = 8192,
    MAX_REQUEST_LINE_SIZE = 1024, // 1kb
    MAX_HEADER_LINE_SIZE = 1024,
};
