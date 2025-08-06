#include "server.h"
#include "dispatcher.h"
#include "request.h"
#include "response.h"
#include "status.h"
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
    int client_fd;
    const server_ctx_t *ctx;
} connection_args_t;

void *handle_connection(void *arg);

void server_start(const int port, char *directory) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    // allows the server to reuse the port immediately after it restarts
    // ensures we don't run into "Already in use" error
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
        0) {
        perror("setsockopt");
        close(server_fd);
        exit(1);
    }

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,

        // sets the port number to 4221, converted to network byte
        // order(big-endian)
        .sin_port = htons(port),

        // binds to all network interface(0.0.0.0)
        .sin_addr = {htonl(INADDR_ANY)},
    };

    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) !=
        0) {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    server_ctx_t server_ctx = {
        .directory = directory,
    };

    int connection_backlog = 5;

    if (listen(server_fd, connection_backlog) != 0) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("Server listening on port %d...\n", port);

    while (1) {
        connection_args_t *args = malloc(sizeof(connection_args_t));
        if (!args) {
            perror("malloc");
            continue;
        }

        args->ctx = &server_ctx;
        args->client_fd = accept(server_fd, NULL, NULL);

        if (args->client_fd < 0) {
            perror("accept");
            free(args);
            continue;
        }

        printf("Client connected (thread: %lu)\n", pthread_self());

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_connection, args) != 0) {
            perror("pthread_create");
            close(args->client_fd);
            free(args);
            continue;
        }

        pthread_detach(thread);
    }
}

void *handle_connection(void *arg) {
    connection_args_t *args = (connection_args_t *)arg;
    int client_fd = args->client_fd;
    const server_ctx_t *ctx = args->ctx;
    free(arg);

    request_t req = {0};
    response_t res = {0};

    request_init(&req);

    if (response_init(&res, ctx) < 0) {
        fprintf(stderr, "Failed to init response (headers alloc failed)\n");
        response_send_status(client_fd, STATUS_INTERNAL_SERVER_ERR);
        return NULL;
    }

    int err = request_parse(client_fd, &req);
    if (err < 0) {
        res.status_code = err == PARSE_ERR_INTERNAL ? STATUS_INTERNAL_SERVER_ERR
                                                    : STATUS_BAD_REQUEST;
        response_send(client_fd, &res);
        goto cleanup;
    }

    dispatcher(&req, &res);

    if (response_send(client_fd, &res) == -1) {
        perror("send_response");
    }

cleanup:
    response_cleanup(&res);
    request_cleanup(&req);
    close(client_fd);

    return NULL;
}
