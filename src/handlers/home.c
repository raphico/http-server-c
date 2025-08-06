#include "handlers/home.h"
#include "headers.h"
#include "status.h"

void handle_home(request_t *req, response_t *res) {
    res->status_code = STATUS_OK;
    headers_add(&res->headers, "Content-Length", "0");
}
