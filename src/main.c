#include "header.h"
#include "request.h"
#include "response.h"
#include "status.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    // Disables output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    socklen_t client_addr_len;
    struct sockaddr_in client_addr;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        goto cleanup;
    }

    // allows the server to reuse the port immediately after it restarts
    // ensures we don't run into "Already in use" error
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
        0) {
        perror("setsockopt");
        goto cleanup;
    }

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,

        // sets the port number to 4221, converted to network byte
        // order(big-endian)
        .sin_port = htons(4221),

        // binds to all network interface(0.0.0.0)
        .sin_addr = {htonl(INADDR_ANY)},
    };

    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) !=
        0) {
        perror("bind");
        goto cleanup;
    }

    int connection_backlog = 5;

    // marks the open socket as ready to receive incoming connection
    if (listen(server_fd, connection_backlog) != 0) {
        perror("listen");
        goto cleanup;
    }

    printf("Waiting for a client to connect...\n");
    client_addr_len = sizeof(client_addr);

    // blocks and wait until a client connect
    int client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0) {
        perror("accept");
        goto cleanup;
    }

    printf("Client connected\n");

    request_t req = {0};
    request_init(&req);

    response_t res = {0};
    if (response_init(&res) < 0) {
        fprintf(stderr, "Failed to init response (headers alloc failed)\n");
        response_send_status(client_fd, STATUS_INTERNAL_SERVER_ERR);
        goto cleanup;
    }

    int result = request_parse(client_fd, &req);
    if (result < 0) {
        if (result == PARSE_ERR_INTERNAL) {
            fprintf(stderr, "Memory allocation failed\n");

            res.status_code = STATUS_INTERNAL_SERVER_ERR;
            response_send(client_fd, &res);
        } else {
            switch (result) {
            case PARSE_ERR_READ:
                fprintf(stderr, "Failed to read from client (I/O error)\n");
                break;
            case PARSE_ERR_CLOSED:
                fprintf(stderr, "Client closed connection unexpectedly\n");
                break;
            case PARSE_ERR_INVALID:
                fprintf(stderr, "Malformed HTTP request\n");
                break;
            default:
                fprintf(stderr, "Unknown parsing error code: %d\n", result);
                break;
            }

            res.status_code = STATUS_BAD_REQUEST,
            response_send(client_fd, &res);
        }
        goto cleanup;
    }

    if (strcmp(req.url, "/") == 0) {
        res.status_code = STATUS_OK;
        headers_add(&res.headers, "Content-Length", "0");
    } else if (strstr(req.url, "echo") != NULL) {
        char *save_ptr;
        char *parts[2];
        int count = 0;

        char *token = strtok_r(req.url, "/", &save_ptr);
        while (token != NULL && count < 2) {
            parts[count++] = token;
            token = strtok_r(NULL, "/", &save_ptr);
        }

        if (count == 2 && strtok_r(NULL, "/", &save_ptr) == NULL) {
            res.status_code = STATUS_OK;

            int len = snprintf(res.body, sizeof(res.body), "%s", parts[1]);
            if (len < 0 || len >= (int)sizeof(res.body)) {
                res.status_code = STATUS_INTERNAL_SERVER_ERR;
            } else {
                res.body_len = (size_t)len;
            }

            char length_str[16];
            snprintf(length_str, sizeof(length_str), "%d", (int)res.body_len);
            headers_add(&res.headers, "Content-Length", length_str);

            headers_add(&res.headers, "Content-Type", "text/plain");
        } else {
            res.status_code = STATUS_BAD_REQUEST;
        }
    } else {
        res.status_code = STATUS_NOT_FOUND;
        headers_add(&res.headers, "Content-Length", "0");
    }

    if (response_send(client_fd, &res) == -1) {
        perror("send_response");
        goto cleanup;
    }

cleanup:
    request_cleanup(&req);
    response_cleanup(&res);
    if (client_fd != -1) {
        close(client_fd);
    }
    if (server_fd != -1) {
        close(server_fd);
    }
    return 0;
}
