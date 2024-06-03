//Vu Van Thang 20215643
//MSSV: 20215643
/*
bai3.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_IP> <server_port> <student_data_file>\n", argv[0]);
        return 1;
    }

    const char* serverIP = argv[1];
    int port = atoi(argv[2]);
    const char* studentDataFile = argv[3];

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
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error connecting to server");
        close(sockfd);
        return 1;
    }

    printf("Connected to server. Enter student information:\n");

    char studentInfo[1024];
    printf("MSSV: ");
    fgets(studentInfo, sizeof(studentInfo), stdin);
    send(sockfd, studentInfo, strlen(studentInfo), 0);

    printf("Full Name: ");
    fgets(studentInfo, sizeof(studentInfo), stdin);
    send(sockfd, studentInfo, strlen(studentInfo), 0);

    printf("Date of Birth: ");
    fgets(studentInfo, sizeof(studentInfo), stdin);
    send(sockfd, studentInfo, strlen(studentInfo), 0);

    printf("Average GPA: ");
    fgets(studentInfo, sizeof(studentInfo), stdin);
    send(sockfd, studentInfo, strlen(studentInfo), 0);

    // Save student data to a file
    FILE* studentDataFilePtr = fopen(studentDataFile, "w");
    if (!studentDataFilePtr) {
        perror("Error opening student data file");
        close(sockfd);
        return 1;
    }
    fprintf(studentDataFilePtr, "%s%s%s%s", argv[1], argv[2], argv[3], studentInfo);
    fclose(studentDataFilePtr);

    printf("Student data saved to %s\n", studentDataFile);

    // Close the socket
    close(sockfd);

    return 0;
}
