//Vu Van Thang 20215643
//MSSV: 20215643
/*
bai4.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <log_file>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    const char* logFile = argv[2];

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

    // Get client IP address
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);

    // Get current time
    time_t currentTime;
    time(&currentTime);
    struct tm* timeInfo = localtime(&currentTime);
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeInfo);

    // Receive data from client
    char clientBuffer[1024];
    recv(clientSock, clientBuffer, sizeof(clientBuffer), 0);

    // Print to console
    printf("%s %s %s\n", clientIP, timeStr, clientBuffer);

    // Write to log file
    FILE* logFilePtr = fopen(logFile, "a");
    if (!logFilePtr) {
        perror("Error opening log file");
        close(clientSock);
        close(sockfd);
        return 1;
    }
    fprintf(logFilePtr, "%s %s %s\n", clientIP, timeStr, clientBuffer);
    fclose(logFilePtr);

    printf("Client data saved to %s\n", logFile);

    // Close sockets
    close(clientSock);
    close(sockfd);

    return 0;
}
