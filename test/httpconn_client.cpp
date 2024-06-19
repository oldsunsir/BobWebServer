#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
/*NOTE: 模仿client建立连接*/

int main(int argc, char** argv){

    int ret = 0;
    char buf[1024] = {0};
    char get_buf[60500] = {0};
    struct stat stat_file = {0};
    const char* file_name = "http_request.txt";

    sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    server_addr.sin_port = htons(8080);

    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);

    ret = connect(socket_fd, (struct sockaddr*)(&server_addr), sizeof(server_addr));
    assert(ret >= 0);

    int request_fd = open(file_name, O_RDONLY);
    assert(request_fd >= 0);
    assert(stat(file_name, &stat_file) == 0);

    ssize_t bytes_read = read(request_fd, buf, stat_file.st_size);

    sleep(5);
    ret = send(socket_fd, buf, stat_file.st_size, 0);
    assert(ret >= 0);
    printf("send ret :%d\n", ret);

    recv(socket_fd, get_buf, sizeof(get_buf)-1, 0);


    printf("%s", get_buf);
    close(socket_fd);
    close(request_fd);
}
