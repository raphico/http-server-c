#include "handlers/home.h"
#include "status.h"

void handle_home(request_t *req, response_t *res) {
    res->status_code = STATUS_OK;
    res->body_len = 0;
}
