#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

int client;

unsigned short send_pasv();
int send_list();
int download_file(char *remote_file);
int upload_file(char *local_file);
int rename_file(char *cur_file, char *new_file);
int delete_file(char *filename);
int print_working_dir();
int change_working_dir(char *dirname);
int make_dir(char *dirname);
int remove_dir(char *dirname);

int main() 
{
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("172.26.64.1");
    addr.sin_port = htons(1234);

    int ret = connect(client, (struct sockaddr *) &addr, sizeof(addr));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    char buf[2048];

    // Nhận chuỗi chào
    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Đăng nhập
    char user[32], pass[32];
    printf("Nhập username: ");
    fgets(user, sizeof(user), stdin);
    printf("Nhập password: ");
    fgets(pass, sizeof(pass), stdin);

    // Xóa ký tự xuống dòng
    user[strlen(user) - 1] = 0;
    pass[strlen(pass) - 1] = 0;

    // Gửi lệnh USER
    sprintf(buf, "USER %s\r\n", user);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Gửi lệnh PASS
    sprintf(buf, "PASS %s\r\n", pass);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Kiểm tra đăng nhập thành công
    if (strncmp(buf, "230 ", 4) == 0) {
        puts("Đăng nhập thành công.");
    } else {
        puts("Đăng nhập thất bại.");
        close(client);
        return 1;
    }

    int choice;
    char filename[256], newname[256], dirname[256];

    while (1) {
        printf("\nHãy chọn chức năng để thực hiện:\n");
        printf("1. In nội dung thư mục hiện tại\n");
        printf("2. Đổi thư mục hiện tại\n");
        printf("3. Tạo thư mục\n");
        printf("4. Xóa thư mục\n");
        printf("5. Download file\n");
        printf("6. Upload file\n");
        printf("7. Đổi tên file\n");
        printf("8. Xóa file\n");
        printf("0. Thoát và ngắt kết nối\n");
        printf("Nhập lựa chọn: ");
        scanf("%d", &choice);
        getchar(); // Xóa ký tự xuống dòng khỏi bộ đệm

        switch (choice) {
            case 1:
                send_list();
                break;
            case 2:
                printf("Nhập tên thư mục để đổi: ");
                fgets(dirname, sizeof(dirname), stdin);
                dirname[strlen(dirname) - 1] = 0; // Xóa ký tự xuống dòng
                change_working_dir(dirname);
                break;
            case 3:
                printf("Nhập tên thư mục để tạo: ");
                fgets(dirname, sizeof(dirname), stdin);
                dirname[strlen(dirname) - 1] = 0; // Xóa ký tự xuống dòng
                make_dir(dirname);
                break;
            case 4:
                printf("Nhập tên thư mục để xóa: ");
                fgets(dirname, sizeof(dirname), stdin);
                dirname[strlen(dirname) - 1] = 0; // Xóa ký tự xuống dòng
                remove_dir(dirname);
                break;
            case 5:
                printf("Nhập tên file để download: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strlen(filename) - 1] = 0; // Xóa ký tự xuống dòng
                download_file(filename);
                break;
            case 6:
                printf("Nhập tên file để upload: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strlen(filename) - 1] = 0; // Xóa ký tự xuống dòng
                upload_file(filename);
                break;
            case 7:
                printf("Nhập tên file hiện tại: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strlen(filename) - 1] = 0; // Xóa ký tự xuống dòng
                printf("Nhập tên file mới: ");
                fgets(newname, sizeof(newname), stdin);
                newname[strlen(newname) - 1] = 0; // Xóa ký tự xuống dòng
                rename_file(filename, newname);
                break;
            case 8:
                printf("Nhập tên file để xóa: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strlen(filename) - 1] = 0; // Xóa ký tự xuống dòng
                delete_file(filename);
                break;
            case 0:
                close(client);
                printf("Đã ngắt kết nối.\n");
                exit(0);
            default:
                printf("Lựa chọn không hợp lệ. Vui lòng thử lại.\n");
        }
    }
    return 0;
}

unsigned short send_pasv() {
    char buf[2048];

    send(client, "PASV\r\n", 6, 0);

    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    char *pos = strchr(buf, '(');
    int i1 = atoi(strtok(pos, "(),"));
    int i2 = atoi(strtok(NULL, "(),"));
    int i3 = atoi(strtok(NULL, "(),"));
    int i4 = atoi(strtok(NULL, "(),"));
    int p1 = atoi(strtok(NULL, "(),"));
    int p2 = atoi(strtok(NULL, "(),"));

    return p1 * 256 + p2;
}

int send_list() {
    unsigned short port = send_pasv();
    printf("Port: %d\n", port);

    int client_data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr_data;
    addr_data.sin_family = AF_INET;
    addr_data.sin_addr.s_addr = inet_addr("172.26.64.1");
    addr_data.sin_port = htons(port);

    int ret = connect(client_data, (struct sockaddr *) &addr_data, sizeof(addr_data));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    send(client, "LIST\r\n", 6, 0);

    char buf[2048];

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    while (1) {
        ret = recv(client_data, buf, sizeof(buf) - 1, 0);
        if (ret <= 0) {
            close(client_data);
            break;
        }

        buf[ret] = 0;
        printf("%s", buf);
    }

    printf("\n");

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    return 0;
}

int download_file(char *remote_file) {
    unsigned short port = send_pasv();
    printf("Port: %d\n", port);

    int client_data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr_data;
    addr_data.sin_family = AF_INET;
    addr_data.sin_addr.s_addr = inet_addr("172.26.64.1");
    addr_data.sin_port = htons(port);

    int ret = connect(client_data, (struct sockaddr *) &addr_data, sizeof(addr_data));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    char buf[2048];

    sprintf(buf, "RETR %s\r\n", remote_file);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    FILE *f = fopen(remote_file, "wb");
    while (1) {
        ret = recv(client_data, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        fwrite(buf, 1, ret, f);
    }
    close(client_data);
    fclose(f);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    return 0;
}

int upload_file(char *local_file) {
    unsigned short port = send_pasv();
    printf("Port: %d\n", port);

    int client_data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr_data;
    addr_data.sin_family = AF_INET;
    addr_data.sin_addr.s_addr = inet_addr("172.26.64.1");
    addr_data.sin_port = htons(port);

    int ret = connect(client_data, (struct sockaddr *) &addr_data, sizeof(addr_data));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    char buf[2048];

    sprintf(buf, "STOR %s\r\n", local_file);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    FILE *f = fopen(local_file, "rb");
    if (f == NULL) {
        perror("fopen() failed");
        return 1;
    }

    while (1) {
        ret = fread(buf, 1, sizeof(buf), f);
        if (ret <= 0)
            break;
        send(client_data, buf, ret, 0);
    }
    close(client_data);
    fclose(f);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    return 0;
}

int rename_file(char *cur_file, char *new_file) {
    char buf[2048];

    sprintf(buf, "RNFR %s\r\n", cur_file);
    send(client, buf, strlen(buf), 0);

    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    sprintf(buf, "RNTO %s\r\n", new_file);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    return 0;
}

int delete_file(char *filename) {
    char buf[2048];

    sprintf(buf, "DELE %s\r\n", filename);
    send(client, buf, strlen(buf), 0);

    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    return 0;
}

int print_working_dir() {
    char buf[2048];

    sprintf(buf, "PWD\r\n");
    send(client, buf, strlen(buf), 0);

    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    return 0;
}

int change_working_dir(char *dirname) {
    char buf[2048];

    sprintf(buf, "CWD %s\r\n", dirname);
    send(client, buf, strlen(buf), 0);

    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    return 0;
}

int make_dir(char *dirname) {
    char buf[2048];

    sprintf(buf, "MKD %s\r\n", dirname);
    send(client, buf, strlen(buf), 0);

    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    return 0;
}

int remove_dir(char *dirname) {
    char buf[2048];

    sprintf(buf, "RMD %s\r\n", dirname);
    send(client, buf, strlen(buf), 0);

    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    return 0;
}
