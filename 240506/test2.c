#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h> // Thư viện cho việc làm việc với thư mục
#include <fcntl.h> // Thư viện cho việc mở file

#define MAX_FILENAME_LEN 256

void *client_proc(void *);

int main() {
    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8002);

    // Gan socket voi cau truc dia chi
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }

    // Chuyen socket sang trang thai cho ket noi
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

    // Gửi danh sách các file trong thư mục cho client
    DIR *dir;
    struct dirent *ent;
    char file_list[2048] = "OK "; // Chuỗi bắt đầu với "OK "
    int num_files = 0;

    if ((dir = opendir(".")) != NULL) {
        // Duyệt qua các file trong thư mục
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) { // Kiểm tra nếu là file thông thường
                strcat(file_list, ent->d_name);
                strcat(file_list, "\r\n"); // Phân cách tên file bằng "\r\n"
                num_files++;
            }
        }
        closedir(dir);
    } else {
        perror("opendir() failed");
        close(client);
        pthread_exit(NULL);
    }

    // Kiểm tra nếu không có file nào trong thư mục
    if (num_files == 0) {
        char error_msg[] = "ERROR No files to download \r\n";
        send(client, error_msg, strlen(error_msg), 0);
        close(client);
        pthread_exit(NULL);
    }

    strcat(file_list, "\r\n"); // Kết thúc danh sách bằng "\r\n\r\n"

    // Chèn số lượng file vào chuỗi
    char num_files_str[10];
    sprintf(num_files_str, "%d", num_files);
    strcat(file_list, num_files_str);
    strcat(file_list, "\r\n");

    // Gửi danh sách file cho client
    send(client, file_list, strlen(file_list), 0);

    // Nhận tên file từ client
    char filename[MAX_FILENAME_LEN];
    int filename_len = recv(client, filename, sizeof(filename), 0);
    if (filename_len <= 0) {
        close(client);
        pthread_exit(NULL);
    }
    filename[filename_len] = '\0';

    // Kiểm tra nếu file tồn tại
    FILE *file = fopen(filename, "rb");
    if (file != NULL) {
        // Gửi tin nhắn "OK N\r\n"
        fseek(file, 0, SEEK_END);
        long filesize = ftell(file);
        fseek(file, 0, SEEK_SET);
        char ok_msg[256];
        sprintf(ok_msg, "OK %ld\r\n", filesize);
        send(client, ok_msg, strlen(ok_msg), 0);

        // Gửi nội dung file cho client
        char buffer[1024];
        size_t read_bytes;
        while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            send(client, buffer, read_bytes, 0);
        }

        // Đóng file và kết nối
        fclose(file);
        close(client);
    } else {
        // Gửi thông báo lỗi và yêu cầu client gửi lại tên file
        char error_msg[] = "ERROR File not found. Please send the filename again.\r\n";
        send(client, error_msg, strlen(error_msg), 0);
        close(client);
    }

    pthread_exit(NULL);
}
