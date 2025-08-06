#pragma once

#include "request.h"

typedef enum {
    ENCODING_IDENTITY,
    ENCODING_GZIP,
} content_encoding_t;

bool supports_gzip(request_t *req);
