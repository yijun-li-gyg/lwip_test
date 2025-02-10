#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <rtems.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <rtems.h>
#include <netstart.h>
#include <arch/sys_arch.h>
#include <lwip/dhcp.h>
#include <rtems/telnetd.h>
#include <arch/sys_arch.h>
//#define CONFIGURE_NETWORKING
//#define CONFIGURE_NETWORKING_DEVICE_NAME "eth0"
//#define CONFIGURE_NETWORKING_DRIVER_NAME "lan8710"

#define BROADCAST_IP "255.255.255.255" // 广播地址
#define PORT 12345                     // 广播端口

const char rtems_test_name[] = "TELNETD 1";
struct netif net_interface;
rtems_shell_env_t env;
static rtems_task Init(rtems_task_argument argument);

#define print_ip( tag, ip ) \
  printf( \
  "%s: %" PRId32 ".%" PRId32 ".%" PRId32 ".%" PRId32 "\n", \
  tag, \
  ( ntohl( ip.addr ) >> 24 ) & 0xff, \
  ( ntohl( ip.addr ) >> 16 ) & 0xff, \
  ( ntohl( ip.addr ) >> 8 ) & 0xff, \
  ntohl( ip.addr ) & 0xff \
  );

static int shell_main_netinfo(
  int    argc,
  char **argv
)
{
  print_ip( "IP", net_interface.ip_addr.u_addr.ip4 );
  print_ip( "Mask", net_interface.netmask.u_addr.ip4 );
  print_ip( "GW", net_interface.gw.u_addr.ip4 );
  return 0;
}

int test()
{
    int sockfd;
    struct sockaddr_in broadcast_addr;
    char *message = "Hello, Broadcast!";

    // 创建UDP套接字
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 设置套接字选项以允许广播
    int broadcast_enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        perror("setsockopt (SO_BROADCAST)");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 设置广播地址和端口
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);
    broadcast_addr.sin_port = htons(PORT);

    // 发送广播消息
    if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
        perror("sendto");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Broadcast message sent.\n");

    close(sockfd);
    return 0;

}

static rtems_task Init(rtems_task_argument argument)
{
  rtems_status_code sc;
  int ret;
  //TEST_BEGIN();
  ip_addr_t ipaddr,netmask,gw;
  unsigned char mac_ethernet_address[] = {0x00, 0x0a, 0x35, 0x00, 0x22, 0x01};
  
  IP_ADDR4(&ipaddr, 169, 254, 1, 255);
  IP_ADDR4(&netmask, 255, 255, 255, 0);
  IP_ADDR4(&gw, 169, 254, 1, 3);
  
  ret = start_networking(&net_interface, &ipaddr, &netmask, &gw, mac_ethernet_address);

    if (ret != 0) {
        return;
    }

    rtems_shell_init_environment();
    test();
 
//   sc = rtems_shell_init(
//        "SHLL",
//        RTEMS_MINIMUM_STACK_SIZE * 4,
//        100,
//        "/dev/console",
//        false,
//        true,
//        NULL);
//    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
    //TEST_END();
    rtems_test_exit(0);
}

#define CONFIGURE_INIT

#define CONFIGURE_MICROSECONDS_PER_TICK 10000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK

#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 32

#define CONFIGURE_SHELL_COMMANDS_INIT
#define CONFIGURE_SHELL_COMMANDS_ALL
//#define CONFIGURE_SHELL_USER_COMMANDS &shell_NETINFO_Command

#define CONFIGURE_MAXIMUM_TASKS 12

#define CONFIGURE_MAXIMUM_POSIX_KEYS 1
#define CONFIGURE_MAXIMUM_SEMAPHORES 20
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 10

//#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT

#define CONFIGURE_UNLIMITED_OBJECTS
#define CONFIGURE_UNIFIED_WORK_AREAS

#include <rtems/shellconfig.h>

#include <rtems/confdefs.h>

