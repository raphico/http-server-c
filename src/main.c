#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

int main() {
    // Disables output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    socklen_t client_addr_len;
    struct sockaddr_in client_addr;

    // opens a TCP socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("Socket creation failed: %s...\n", strerror(errno));
        return 1;
    }

    // allows the server to reuse the port immediately after it restarts
    // ensures we don't run into "Already in use" error
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        printf("SO_REUSEADDR failed: %s...\n", strerror(errno));
        return 1;
    }

    // defines an IPV4 socket address
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET, // sets the address family as IPV4
        .sin_port = htons(4221), // sets the port number to 4221, converted to network byte order(big-endian)
        .sin_addr = { htonl(INADDR_ANY) }, // binds to all network interface(0.0.0.0)
    };

    // binds the socket to IP + port
    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
        printf("bind failed: %s...\n", strerror(errno));
        return 1;
    }

    // sets the maximum number of pending connections
    int connection_backlog = 5;

    // marks the open socket as passive, meaning it is ready to receive incoming client connections
    if (listen(server_fd, connection_backlog) != 0) {
        printf("listen failed: %s...\n", strerror(errno));
        return 1;
    }

    printf("Waiting for a client to connect...\n");
    client_addr_len = sizeof(client_addr);

    // blocks and wait until a client connect
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0) {
        printf("accept failed: %s...\n", strerror(errno));
        return 1;
    }

	printf("Client connected\n");

	char *response = "HTTP/1.1 200 OK\r\n\r\n";
	ssize_t total_sent = 0;
	ssize_t response_len = strlen(response);

	while (total_sent < response_len) {
		ssize_t sent = write(client_fd, response + total_sent, response_len - total_sent);
		if (sent < 0) {
			printf("write failed: %s...\n", strerror(errno));
			break;
		}

		total_sent += sent;
	}

    close(server_fd);
    close(client_fd);
    
    return 0;
}