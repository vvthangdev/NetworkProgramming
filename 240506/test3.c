#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <fcntl.h>

#define MAX_FILENAME_LEN 256

void *client_proc(void *);

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

    while (1) {
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted, client = %d\n", client);
        
        pthread_t tid;
        pthread_create(&tid, NULL, client_proc, &client);
        pthread_detach(tid);
    }

    return 0;
}

void *client_proc(void *arg) {
    int client = *(int *)arg;

    DIR *dir;
    struct dirent *ent;
    char file_list[2048] = "OK ";
    int num_files = 0;

    if ((dir = opendir(".")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) {
                strcat(file_list, ent->d_name);
                strcat(file_list, "\r\n");
                num_files++;
            }
        }
        closedir(dir);
    } else {
        perror("opendir() failed");
        close(client);
        pthread_exit(NULL);
    }

    if (num_files == 0) {
        char error_msg[] = "ERROR No files to download \r\n";
        send(client, error_msg, strlen(error_msg), 0);
        close(client);
        pthread_exit(NULL);
    }

    strcat(file_list, "\r\n");
    char num_files_str[10];
    sprintf(num_files_str, "%d", num_files);
    strcat(file_list, num_files_str);
    strcat(file_list, "\r\n");
    file_list[strlen(file_list)] = '\0';

    send(client, file_list, strlen(file_list), 0);

    char filename[MAX_FILENAME_LEN];
    int filename_len = recv(client, filename, sizeof(filename), 0);
    if (filename_len <= 0) {
        close(client);
        pthread_exit(NULL);
    }
    filename[filename_len] = '\0';

    FILE *file = fopen(filename, "rb");
    if (file != NULL) {
        fseek(file, 0, SEEK_END);
        long filesize = ftell(file);
        fseek(file, 0, SEEK_SET);
        char ok_msg[64];
        snprintf(ok_msg, sizeof(ok_msg), "OK %ld\r\n", filesize);
        send(client, ok_msg, strlen(ok_msg), 0);

        // Gửi nội dung tệp
        int sent_bytes = sendfile(client, fileno(file), NULL, filesize);
        if (sent_bytes == -1) {
            perror("sendfile() failed");
        }

        fclose(file); // Đóng tệp
    } else {
        // Gửi thông báo lỗi và yêu cầu client gửi lại tên file
        char error_msg[] = "ERROR File not found \r\n";
        send(client, error_msg, strlen(error_msg), 0);
    }

    close(client); // Đóng kết nối
    pthread_exit(NULL);
}
