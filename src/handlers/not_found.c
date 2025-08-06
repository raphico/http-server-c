#include "handlers/not_found.h"
#include "status.h"

void handle_not_found(request_t *req, response_t *res) {
    res->status_code = STATUS_NOT_FOUND;
    headers_add(&res->headers, "Content-Length", "0");
}
