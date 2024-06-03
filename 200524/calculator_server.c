/*
Vu Van Thang 20215643
Problem: calculator_server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>

int port;

void signal_handler(int signo)
{
    wait(NULL);
}

void send_response(int clientSocket, const char* content)
{
    char header[1024];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n");
    send(clientSocket, header, strlen(header), 0);
    send(clientSocket, content, strlen(content), 0);
}

void handle_calculation(int clientSocket, double a, double b, const char* cmd)
{
    char response[1024];
    double result;

    if (strcmp(cmd, "add") == 0) {
        result = a + b;
    } else if (strcmp(cmd, "sub") == 0) {
        result = a - b;
    } else if (strcmp(cmd, "mul") == 0) {
        result = a * b;
    } else if (strcmp(cmd, "div") == 0) {
        if (b != 0) {
            result = a / b;
        } else {
            send_response(clientSocket, "<html><h1>Error: Division by zero</h1></html>");
            return;
        }
    } else {
        send_response(clientSocket, "<html><h1>Error: Unknown command</h1></html>");
        return;
    }

    sprintf(response, "<html><h1>Result: %.2f</h1></html>", result);
    send_response(clientSocket, response);
}

void *client_thread(void *param)
{
    int clientSocket = *(int *)param;
    char buff[2048];
    int ret = recv(clientSocket, buff, sizeof(buff) - 1, 0);
    if (ret <= 0) {
        close(clientSocket);
        return NULL;
    }
    buff[ret] = 0;

    if (strncmp(buff, "GET /calculate", 14) == 0) {
        char *query = strchr(buff, '?') + 1;
        char *a_str = strstr(query, "a=") + 2;
        char *b_str = strstr(query, "b=") + 2;
        char *cmd_str = strstr(query, "cmd=") + 4;

        double a = atof(a_str);
        double b = atof(b_str);
        char *cmd = strtok(cmd_str, " ");

        handle_calculation(clientSocket, a, b, cmd);
    } else if (strncmp(buff, "POST /calculate", 15) == 0) {
        char *content = strstr(buff, "\r\n\r\n") + 4;
        char *a_str = strstr(content, "a=") + 2;
        char *b_str = strstr(content, "b=") + 2;
        char *cmd_str = strstr(content, "cmd=") + 4;

        double a = atof(a_str);
        double b = atof(b_str);
        char *cmd = strtok(cmd_str, "&");

        handle_calculation(clientSocket, a, b, cmd);
    } else {
        const char *response =
            "<html>"
            "<body>"
            "<form action='/calculate' method='get'>"
            "a: <input type='text' name='a'><br>"
            "b: <input type='text' name='b'><br>"
            "cmd: <select name='cmd'>"
            "<option value='add'>add</option>"
            "<option value='sub'>sub</option>"
            "<option value='mul'>mul</option>"
            "<option value='div'>div</option>"
            "</select><br>"
            "<input type='submit' value='Calculate'>"
            "</form>"
            "<form action='/calculate' method='post'>"
            "a: <input type='text' name='a'><br>"
            "b: <input type='text' name='b'><br>"
            "cmd: <select name='cmd'>"
            "<option value='add'>add</option>"
            "<option value='sub'>sub</option>"
            "<option value='mul'>mul</option>"
            "<option value='div'>div</option>"
            "</select><br>"
            "<input type='submit' value='Calculate'>"
            "</form>"
            "</body>"
            "</html>";
        send_response(clientSocket, response);
    }

    close(clientSocket);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Missing arguments\n");
        exit(1);
    }
    port = atoi(argv[1]);

    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == -1) {
        perror("Create socket failed");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Binding failed");
        exit(1);
    }

    if (listen(serverSocket, 5) == -1) {
        perror("Listening failed");
        exit(1);
    }
    printf("Waiting for client connecting ...\n");

    signal(SIGPIPE, signal_handler);

    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLength = sizeof(clientAddr);
        int client = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);
        if (client == -1) {
            perror("Accept failed");
            continue;
        }
        printf("Client connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    close(serverSocket);
    printf("Socket closed\n");
    return 0;
}
