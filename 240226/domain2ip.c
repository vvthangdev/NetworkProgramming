#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>

int main(int argc, char *argv[]) {
    struct addrinfo *res, *p;
    int ret = getaddrinfo(argv[1], "http", NULL, &res);
    if (ret != 0) {
        printf("Khong phan giai duoc\n");
        return 1;
    }

    // Duyet danh sach ket qua
    p = res;
    while (p != NULL) {
        if (p->ai_family == AF_INET) {
            printf("IPv4\n");
            struct sockaddr_in addr;
            memcpy(&addr, p->ai_addr, sizeof(addr));
            printf("IP: %s\n", inet_ntoa(addr.sin_addr));
        } else if (p->ai_family == AF_INET6) {
            printf("IPv6\n");
        } 
        p = p->ai_next;
    }
}