#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>

int main() {
    struct in_addr addr; // IPv4 address structure
    struct hostent *host;

    // Example IP address in network byte order
    addr.s_addr = inet_addr("127.0.0.1"); // Example IP address

    // Call gethostbyaddr to convert IP address to hostname
    host = gethostbyaddr((const char *)&addr, sizeof(addr), AF_INET);
    if (host == NULL) {
        herror("gethostbyaddr");
        return 1;
    }

    printf("Hostname: %s\n", host->h_name);

    return 0;
}
