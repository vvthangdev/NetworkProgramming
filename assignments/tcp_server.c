//Vu Van Thang 20215643
//MSSV: 20215643
/*
bai2.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <port> <greeting_file> <client_data_file>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    const char* greetingFile = argv[2];
    const char* clientDataFile = argv[3];

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Error creating socket");
        return 1;
    }

    // Set up server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error binding socket");
        close(sockfd);
        return 1;
    }

    // Listen for incoming connections
    if (listen(sockfd, 5) == -1) {
        perror("Error listening");
        close(sockfd);
        return 1;
    }

    printf("Server listening on port %d...\n", port);

    // Accept incoming connection
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int clientSock = accept(sockfd, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientSock == -1) {
        perror("Error accepting connection");
        close(sockfd);
        return 1;
    }

    // Read greeting from file
    FILE* greetingFilePtr = fopen(greetingFile, "r");
    if (!greetingFilePtr) {
        perror("Error opening greeting file");
        close(clientSock);
        close(sockfd);
        return 1;
    }

    char greetingBuffer[1024];
    fgets(greetingBuffer, sizeof(greetingBuffer), greetingFilePtr);
    fclose(greetingFilePtr);

    // Send greeting to client
    send(clientSock, greetingBuffer, strlen(greetingBuffer), 0);

    // Receive data from client
    char clientBuffer[1024];
    FILE* clientDataFilePtr = fopen(clientDataFile, "w");
    while (1) {
        int bytesRead = recv(clientSock, clientBuffer, sizeof(clientBuffer), 0);
        if (bytesRead <= 0) {
            break;
        }
        fwrite(clientBuffer, 1, bytesRead, clientDataFilePtr);
    }
    fclose(clientDataFilePtr);

    printf("Client data saved to %s\n", clientDataFile);

    // Close sockets
    close(clientSock);
    close(sockfd);

    return 0;
}
