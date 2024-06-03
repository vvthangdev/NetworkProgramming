#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

int main() 
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("10.13.32.202");
    addr.sin_port = htons(21);

    int ret = connect(client, (struct sockaddr *) &addr, sizeof(addr));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    char buf[2048];

    // Nhan xau chao
    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Dang nhap
    char user[32], pass[32];
    printf("Nhap username: ");
    fgets(user, sizeof(user), stdin);
    printf("Nhap password: ");
    fgets(pass, sizeof(pass), stdin);

    // Xoa ky tu xuong trong
    user[strlen(user) - 1] = 0;
    pass[strlen(pass) - 1] = 0;

    // Gui lenh USER
    sprintf(buf, "USER %s\r\n", user);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Gui lenh PASS
    sprintf(buf, "PASS %s\r\n", pass);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Kiem tra dang nhap thanh cong
    if (strncmp(buf, "230 ", 4) == 0) {
        puts("Dang nhap thanh cong.");
    } else {
        puts("Dang nhap that bai.");
        close(client);
        return 1;
    }

    // Dang nhap thanh cong. Lay ve danh sach thu muc va tap tin.
    // Gui lenh PASV
    send(client, "PASV\r\n", 6, 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Xu ly ket qua lenh PASV
    // 1. Lay ra dia chi IP
    // 2. Lay ra gia tri cong
    // 3. Mo ket noi den dia chi tren server

    // 227 Entering Passive Mode (10,13,32,202,201,21)
    char *pos = strchr(buf, '(');
    int i1 = atoi(strtok(pos, "(),"));
    int i2 = atoi(strtok(NULL, "(),"));
    int i3 = atoi(strtok(NULL, "(),"));
    int i4 = atoi(strtok(NULL, "(),"));
    int p1 = atoi(strtok(NULL, "(),"));
    int p2 = atoi(strtok(NULL, "(),"));

    unsigned short port = p1 * 256 + p2;

    printf("IP: %d.%d.%d.%d\n", i1, i2, i3, i4);
    printf("Port: %d\n", p1 * 256 + p2);

    // Mo ket noi du lieu 
    int client_data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr_data;
    addr_data.sin_family = AF_INET;
    addr_data.sin_addr.s_addr = inet_addr("10.13.32.202");
    addr_data.sin_port = htons(port);

    ret = connect(client_data, (struct sockaddr *) &addr_data, sizeof(addr_data));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    // Gui lenh LIST
    send(client, "LIST\r\n", 6, 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Nhan du lieu tren kenh du lieu
    // In ra man hinh
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

    // Download file
    printf("Nhap ten file de download: ");
    char filename[256];
    fgets(filename, sizeof(filename), stdin);
    filename[strlen(filename) - 1] = 0;

    // Gui lenh PASV
    send(client, "PASV\r\n", 6, 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Xu ly ket qua lenh PASV
    // 1. Lay ra dia chi IP
    // 2. Lay ra gia tri cong
    // 3. Mo ket noi den dia chi tren server

    pos = strchr(buf, '(');
    i1 = atoi(strtok(pos, "(),"));
    i2 = atoi(strtok(NULL, "(),"));
    i3 = atoi(strtok(NULL, "(),"));
    i4 = atoi(strtok(NULL, "(),"));
    p1 = atoi(strtok(NULL, "(),"));
    p2 = atoi(strtok(NULL, "(),"));

    port = p1 * 256 + p2;

    printf("IP: %d.%d.%d.%d\n", i1, i2, i3, i4);
    printf("Port: %d\n", p1 * 256 + p2);

    // Mo ket noi du lieu 
    client_data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    addr_data.sin_family = AF_INET;
    addr_data.sin_addr.s_addr = inet_addr("10.13.32.202");
    addr_data.sin_port = htons(port);

    ret = connect(client_data, (struct sockaddr *) &addr_data, sizeof(addr_data));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    // Gui lenh retr
    sprintf(buf, "retr %s\r\n", filename);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Nhan noi dung file
    FILE *f = fopen(filename, "wb");
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

    close(client);
}