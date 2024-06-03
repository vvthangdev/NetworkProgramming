#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

#define MAX_FDS 2048

void encodeString(char *str) {
    while (*str) {
        if ((*str >= 'A' && *str < 'Z') || (*str >= 'a' && *str < 'z'))
            (*str)++;
        else if (*str == 'Z')
            *str = 'A';
        else if (*str == 'z')
            *str = 'a';
        else if (*str >= '0' && *str <= '9')
            *str = '9' - *str + '0';
        str++;
    }
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5)) {
        perror("listen() failed");
        return 1;
    }

    struct pollfd fds[MAX_FDS];
    int nfds = 0;

    fds[0].fd = listener;
    fds[0].events = POLLIN;
    nfds++;

    char buf[256];

    while (1) {
        int ret = poll(fds, nfds, -1);
        if (ret == -1) {
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == listener) {
                    int client = accept(listener, NULL, NULL);
                    if (client >= MAX_FDS) {
                        close(client);
                    } else {
                        fds[nfds].fd = client;
                        fds[nfds].events = POLLIN;
                        nfds++;

                        printf("New client connected: %d\n", client);

                        // Send greeting message
                        char greeting[50];
                        sprintf(greeting, "Xin chao. Hien co %d clients dang ket noi.\n", nfds - 1);
                        send(client, greeting, strlen(greeting), 0);
                    }
                } else {
                    int client = fds[i].fd;
                    ret = recv(client, buf, sizeof(buf), 0);
                    if (ret <= 0) {
                        // Connection closed
                        printf("Client %d disconnected\n", client);
                        close(client);
                        // Remove from pollfd array
                        for (int j = i; j < nfds - 1; j++) {
                            fds[j] = fds[j + 1];
                        }
                        nfds--;
                    } else {
                        buf[ret] = '\0';
                        printf("Received from %d: %s\n", client, buf);

                        // Check for exit command
                        if (strcmp(buf, "exit\n") == 0) {
                            send(client, "Tam biet!\n", 10, 0);
                            close(client);
                            // Remove from pollfd array
                            for (int j = i; j < nfds - 1; j++) {
                                fds[j] = fds[j + 1];
                            }
                            nfds--;
                        } else {
                            // Encode string
                            encodeString(buf);
                            send(client, buf, strlen(buf), 0);
                        }
                    }
                }
            }
        }
    }

    return 0;
}
