#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <rtems.h>

#define NTP_PORT 123
#define NTP_SERVER "169.254.1.1"
#define NTP_PACKET_SIZE 48
#define NTP_TIMESTAMP_DELTA 2208988800ull // 从1900年到1970年的秒数

rtems_task Init(
  rtems_task_argument ignored
)
{
// 获取NTP服务器时间
time_t get_ntp_time(const char *server) {
    int sockfd;
    struct sockaddr_in server_addr;
    char ntp_packet[NTP_PACKET_SIZE];
    time_t ntp_time = 0;

    // 创建UDP套接字
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket");
        return 0;
    }

    // 设置NTP服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(NTP_PORT);
    if (inet_pton(AF_INET, server, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return 0;
    }

    // 初始化NTP数据包
    memset(ntp_packet, 0, NTP_PACKET_SIZE);
    ntp_packet[0] = 0x1B; // 标志位：版本3，客户端模式

    // 发送NTP请求
    if (sendto(sockfd, ntp_packet, NTP_PACKET_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("sendto");
        close(sockfd);
        return 0;
    }

    // 接收NTP响应
    if (recv(sockfd, ntp_packet, NTP_PACKET_SIZE, 0) < 0) {
        perror("recv");
        close(sockfd);
        return 0;
    }

    // 解析NTP时间戳
    ntp_time = (time_t)(ntohl(((uint32_t *)ntp_packet)[10]) - NTP_TIMESTAMP_DELTA);

    close(sockfd);
    return ntp_time;
}


int main() {
    const char *ntp_server = NTP_SERVER;
    time_t ntp_time, local_time;
    double time_difference;

    // 获取NTP服务器时间
    ntp_time = get_ntp_time(ntp_server);
    if (ntp_time == 0) {
        fprintf(stderr, "Failed to get NTP time.\n");
        return 1;
    }

    // 获取本地系统时间
    local_time = time(NULL);

    // 计算时间差
    time_difference = difftime(local_time, ntp_time);

    // 输出结果
    printf("NTP Server Time (%s): %s", ntp_server, ctime(&ntp_time));
    printf("Local System Time: %s", ctime(&local_time));
    printf("Time Difference: %.6f seconds\n", time_difference);

    // 判断时间是否准确
    if (fabs(time_difference) < 1.0) {
        printf("Time is accurate (difference < 1 second).\n");
    } else {
        printf("Time is NOT accurate (difference >= 1 second).\n");
    }

    return 0;
}
}