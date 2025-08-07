#include "handlers/echo.h"
#include "compression.h"
#include "headers.h"
#include "status.h"
#include <stdio.h>
#include <string.h>

void handle_echo(request_t *req, response_t *res) {
    const char *echo = req->url + 6; // skips "/echo/"

    int n = snprintf(res->body, sizeof(res->body), "%s", echo);
    if (n < 0 || n >= (int)sizeof(res->body)) {
        res->status_code = STATUS_INTERNAL_SERVER_ERR;
        res->body_len = 0;
        return;
    }

    res->content_encoding =
        supports_gzip(req) ? ENCODING_GZIP : ENCODING_IDENTITY;

    res->status_code = STATUS_OK;
    res->body_len = (size_t)n;

    headers_add(&res->headers, "Content-Type", "text/plain");
}
