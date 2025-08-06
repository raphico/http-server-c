#include "server.h"
#include "util.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Disables output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    char *directory = "/tmp";

    if (argc == 1) {
        // No arguments — use default
    } else if (argc == 3 && strcmp(argv[1], "--directory") == 0) {
        directory = argv[2];
    } else {
        print_usage();
        return 1;
    }

    server_start(4221, directory);
    return 0;
}
