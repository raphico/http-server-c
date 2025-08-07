#pragma once

#include "request.h"
#include "response.h"

bool supports_gzip(request_t *req);
int compress_body(response_t *res);
