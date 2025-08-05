#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>

typedef struct {
    char *method;
    char *url;
} request;

enum HttpStatus {
    STATUS_OK = 200,
    STATUS_NOT_FOUND = 404,
    STATUS_BAD_REQUEST = 400,
    STATUS_INTERNAL_SERVER_ERR = 500,
};

enum ParseError {
    PARSE_ERR_READ = -1,
    PARSE_ERR_CLOSED = -2,
    PARSE_ERR_INVALID = -3,
    PARSE_ERR_INTERNAL = -4,
};

int parse_request(int fd, request *req);
int send_response(int fd, int status_code);
const char *status_text(int status_code);
void cleanup(request *req);
__attribute__((noreturn)) void panic(const char *msg);

int main() {
    // Disables output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    socklen_t client_addr_len;
    struct sockaddr_in client_addr;

    // opens a TCP socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        goto cleanup;
    }

    // allows the server to reuse the port immediately after it restarts
    // ensures we don't run into "Already in use" error
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        goto cleanup;
    }

    // defines an IPV4 socket address
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET, // sets the address family as IPV4
        .sin_port = htons(4221), // sets the port number to 4221, converted to network byte order(big-endian)
        .sin_addr = { htonl(INADDR_ANY) }, // binds to all network interface(0.0.0.0)
    };

    // binds the socket to IP + port
    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
        perror("bind");
        goto cleanup;
    }

    // sets the maximum number of pending connections
    int connection_backlog = 5;

    // marks the open socket as passive, meaning it is ready to receive incoming client connections
    if (listen(server_fd, connection_backlog) != 0) {
        perror("listen");
        goto cleanup;
    }

    printf("Waiting for a client to connect...\n");
    client_addr_len = sizeof(client_addr);

    // blocks and wait until a client connect
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0) {
        perror("accept");
        goto cleanup;
    }

	printf("Client connected\n");

    request req = {0};
    int result = parse_request(client_fd, &req);
    if (result < 0) {
         if (result == PARSE_ERR_INTERNAL) {
            fprintf(stderr, "Memory allocation failed\n");
            send_response(client_fd, STATUS_INTERNAL_SERVER_ERR);
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
            send_response(client_fd, STATUS_BAD_REQUEST);
        }
        goto cleanup;
    }


    int status_code = (strcmp(req.url, "/") == 0) ? STATUS_OK : STATUS_NOT_FOUND;
    if (send_response(client_fd, status_code) == -1) {
        perror("send_response");
        goto cleanup;
    }

    cleanup:
        cleanup(&req);
        if (client_fd != -1) close(client_fd);
        if (server_fd != -1) close(server_fd);
        return 0;
}

int send_response(int fd, int status_code) {
    char buf[1024];
    int len = snprintf(
        buf, sizeof(buf),
        "HTTP/1.1 %d %s\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code,
        status_text(status_code)
    );
    if (len < 0 || len >= sizeof(buf)) {
        return -1;
    }

	ssize_t total_sent = 0;
	while (total_sent < len) {
        ssize_t sent = send(fd, buf + total_sent, len - total_sent, MSG_NOSIGNAL);
		if (sent < 0) {
            return -1;
		}

		total_sent += sent;
	}

    return 0;
}

const char *status_text(int status_code) {
    switch (status_code) {
        case STATUS_OK: return "OK";
        case STATUS_NOT_FOUND: return "Not Found";
        case STATUS_BAD_REQUEST: return "Bad Request";
        case STATUS_INTERNAL_SERVER_ERR: return "Internal Server Error";
        default: panic("Unexpected status code");
    }
}

int parse_request(int fd, request *req) {
    char request_line[4096];
    ssize_t n;
    int i = 0;

    while (i < (int)(sizeof(request_line) - 1)) {
        char ch;
        n = recv(fd, &ch, 1, 0);
        if (n < 0) {
            perror("recv");
            return PARSE_ERR_READ;
        }

        if (n == 0) {
            return PARSE_ERR_CLOSED;
        }

        if (ch == '\r') continue;
        if (ch == '\n') break;

        request_line[i++] = ch;
    }

    request_line[i] = '\0';

    char *save_ptr;
    char *method = strtok_r(request_line, " ", &save_ptr);
    char *url = strtok_r(NULL, " ", &save_ptr);

    if (!method || !url) {
        return PARSE_ERR_INVALID;
    }

    req->method = strdup(method);
    req->url = strdup(url);
    if (!req->method || !req->url) {
        cleanup(req);
        return PARSE_ERR_INTERNAL;
    }

    return 0;
}

void cleanup(request *req) {
    free(req->method);
    free(req->url);
    
    req->method = NULL;
    req->url = NULL;
}

void panic(const char *msg) {
    fprintf(stderr, "PANIC: %s\n", msg);
    abort();
}
