#pragma once

#include "request.h"
#include "response.h"

void handle_get_file(request_t *req, response_t *res);
void handle_post_file(request_t *req, response_t *res);
