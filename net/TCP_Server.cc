#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUF_SIZE 1024

int main()
{
    int sockfd, new_fd;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE] = {0};

    // 1. 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket create fail");
        exit(1);
    }

    // 配置服务器地址结构
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // 2. 绑定端口
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind fail");
        close(sockfd);
        exit(1);
    }

    // 3. 开始监听
    listen(sockfd, 5);
    printf("TCP 回声服务器启动，端口 %d 监听中...\n", PORT);

    while (1)
    {
        // 4. 阻塞等待客户端连接
        new_fd = accept(sockfd, NULL, NULL);
        if (new_fd < 0)
        {
            perror("accept fail");
            continue;
        }
        printf("客户端已连接\n");

        // 🔥 关键修改：循环收发，不断开连接
        while (1)
        {
            ssize_t n = read(new_fd, buffer, BUF_SIZE);
            
            // 客户端断开 或 出错
            if (n <= 0)
            {
                break;
            }

            printf("收到客户端数据: %s\n", buffer);
            write(new_fd, buffer, n); // 回声
            memset(buffer, 0, BUF_SIZE);
        }

        // 🔥 只有客户端断开时才会走到这里
        close(new_fd);
        printf("客户端连接断开\n");
    }

    close(sockfd);
    return 0;
}