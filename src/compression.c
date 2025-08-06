#include "compression.h"
#include "headers.h"
#include <string.h>

bool supports_gzip(request_t *req) {
    char *accept_encoding = headers_get(&req->headers, "accept-encoding");
    return accept_encoding && strstr(accept_encoding, "gzip");
}
