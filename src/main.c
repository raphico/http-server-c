#include "server.h"
#include <stdio.h>

int main() {
    // Disables output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    server_start(4221);
    return 0;
}
