#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <rtems.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <rtems.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <netstart.h>
#include <arch/sys_arch.h>
#include <lwip/dhcp.h>
#include <rtems/telnetd.h>
#include <arch/sys_arch.h>
#include <stdarg.h>
#include <rtems/malloc.h> 
#include <rtems/printer.h>
#include <rtems/bspIo.h>  // 用于 printk 或 BSP 输出
//#define LWIP_IPV6                   1
#define PORT 22222  // 服务器端口
//#define DEST_ADDR "fe80::2059:a2ff:fee7:6359" // 目标地址
//#define DEST_ADDR "fe80::2059:a2ff:fee7:6357"
//#define DEST_ADDR "fe80::9489:55ff:fe1e:8938"
#define DEST_ADDR "fe80::1c93:caff:fe9b:8852"
//#define DEST_ADDR "fe80::215:5dff:fe43:d8eb"
//#define DEST_ADDR "fe80::215:5dff:fe43:d346"
//#define DEST_ADDR "fe80::d89f:88ff:fe0d:fb90"
//#define DEST_ADDR "2001::/16"
//#define DEST_ADDR "::1/128"
#define LWIP_UDP 1
#define LWIP_TCP 1
static const char send_data[] = "This is UDP Client from RT-Thread.";
const char rtems_test_name[] = "TELNETD 1";
struct netif net_interface;
rtems_shell_env_t env;
static rtems_task Init(rtems_task_argument argument);

int test(){
    int sockfd, n;
    struct sockaddr_in6 server_addr;
    char buffer[1024];
    int opt = 1;

    // 创建IPv6的TCP套接字
    if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) == -1) {
    // 创建IPv6的UDP套接字
   // if ((sockfd = socket(PF_INET6, SOCK_DGRAM, 0)) == -1){
      
        perror("创建套接字失败");
        exit(EXIT_FAILURE);
    }

    // 设置可以重用地址和端口以提高复用性
    if (setsockopt(sockfd,  SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
   // if (setsockopt(sockfd, SOL_SOCKET , SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("设置参数失败");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    struct sockaddr_in6 servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_addr = in6addr_any; // 目的地的填写,如果是本地测试即可填::1或者 ::
   //servaddr.sin6_addr = "{::1}";
  //  servaddr.sin6_port = htons(PORT);
  servaddr.sin6_port =htons(PORT);
    server_addr.sin6_scope_id = if_nametoindex("rtap");
    printf("AF_INET6%d\n",AF_INET6);
    printf("port%ld\n",ntohs(servaddr.sin6_port));
     printk("Here is sender.c\n");
    // 创建的客户端用inet_pton来填入服务器的IPv6地址，如果服务器在一个 LAN 距离，则可以用ANY,或者127.0.0.1等，这边先使用::1
    if (inet_pton(AF_INET6, DEST_ADDR, &servaddr.sin6_addr) <= 0) {
        perror("无效的服务器地址");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
   
    // 在客户端connect前添加协议族强制校验
/*    
    int domain = AF_UNSPEC;
    socklen_t len = sizeof(domain);
    getsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &domain, &len);
    if (domain != AF_INET6) {
        printf("协议栈错误! 实际协议族: %d\n", domain);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
*/

    char buf[1024];
    inet_ntop(AF_INET6, &servaddr.sin6_addr, buf, sizeof(buf));
    printf("%s\n", buf);
    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        printk("PORT",PORT);
        perror("连接失败");
        close(sockfd);
        //exit(EXIT_FAILURE);
    }

    printf("已连接到服务器%U:%d\n", servaddr.sin6_addr, ntohs(servaddr.sin6_port));

    while(1)
    {
    // 发送数据到服务器
    char *message = "Hello, Server!";
    n = send(sockfd, message, strlen(message), 0);
    //n = sendto(sockfd,message,strlen(message), 0,(struct sockaddr *)&servaddr,sizeof(servaddr));
    if (n < 0) {
        perror("发送失败");
       // close(sockfd);
        //exit(EXIT_FAILURE);
    }

    // 接收服务器的响应
    int received = recv(sockfd, buffer, sizeof(buffer), 0);
    if (received < 0) {
        perror("接收失败");
        break;
      //  close(sockfd);
        //exit(EXIT_FAILURE);
    } else if (received > 0) {
        buffer[received] = '\0';
        printf("接收服务器响应: %s\n", buffer);
    }
}
    close(sockfd); // 关闭已连接的套接字
    return 0;
}
static rtems_task Init(rtems_task_argument argument)
{
  rtems_status_code sc;
  int ret;
  //TEST_BEGIN();
  ip_addr_t ipaddr,netmask,gw;
  unsigned char mac_ethernet_address[] = {0x00, 0x0a, 0x35, 0x00, 0x22, 0x01};
 
  
  //IP_ADDR4(&ipaddr, 169, 254, 1, 2);
  //IP_ADDR4(&netmask, 255, 255, 255, 0);
  //IP_ADDR4(&gw, 169, 254, 1, 1);
  //IP_ADDR6(&ipaddr, 169, 254, 1, 101);
  //IP_ADDR6(&netmask,255, 255, 0 ,0);
  //IP_ADDR6(&gw, 169, 254, 1, 1);
  /* IPv6地址配置 */
   // IP_ADDR6_HOST(&ipaddr,  0xfe80, 0x9489, 0x55ff, 0xfe1e, 0x8938);  // 2001:db8::1
   // IP_ADDR6_HOST(&netmask, 0xffff, 0xffff, 0xffff, 0xffff, 0, 0, 0, 0); // /64
   // IP_ADDR6_HOST(&gw,      0xfe80, 0x9489, 0x55ff, 0xfe1e, 0x8939);  // 2001:db8::2
/*
    IP_ADDR6_HOST(
      &ipaddr,                // 目标地址变量
      PP_HTONL(0xfe800000),   // Block1: fe80:: 
      PP_HTONL(0x00000000),   // Block2: 0000:0000
      PP_HTONL(0x948955ff),   // Block3: 9489:55ff
      PP_HTONL(0xfe1e8938)    // Block4: fe1e:8938
    );
    IP_ADDR6_HOST(
      &netmask,                // 目标地址变量
      PP_HTONL(0xffffffff),   // Block1: fe80:: 
      PP_HTONL(0xffffffff),   // Block2: 0000:0000
      PP_HTONL(0x00000000),   // Block3: 9489:55ff
      PP_HTONL(0x00000000)    // Block4: fe1e:8938
    );
    IP_ADDR6_HOST(
      &gw,                // 目标地址变量
      PP_HTONL(0xfe800000),   // Block1: fe80:: 
      PP_HTONL(0x00000000),   // Block2: 0000:0000
      PP_HTONL(0x948955ff),   // Block3: 9489:55ff
      PP_HTONL(0xfe1e8939)    // Block4: fe1e:8938
    );
*/
/*
IP_ADDR6(
      &ipaddr,                // 目标地址变量
      0xfe800000,   // Block1: fe80:: 
      0x00000000,   // Block2: 0000:0000
      0x1c93caff,   // Block3: 9489:55ff
      0xfe9b8850    // Block4: fe1e:8938
    );
    IP_ADDR6(
      &netmask,                // 目标地址变量
      0xffffffff,   // Block1: ffff:ffff
      0xffffffff,   // Block2: ffff:ffff
      0x00000000,   // Block3: 0000:0000
      0x00000000    // Block4: 0000:0000
    );
    IP_ADDR6(
      &gw,                // 目标地址变量
      0xfe800000,   // Block1: fe80:: 
      0x00000000,   // Block2: 0000:0000
      0x1c93caff,   // Block3: 9489:55ff
      0xfe9b8852    // Block4: fe1e:8939
    );
    
    */
    IP_ADDR6(
      &ipaddr,                // 目标地址变量
      PP_HTONL(0xfe800000),   // Block1: fe80:: 
      PP_HTONL(0x00000000),   // Block2: 0000:0000
      PP_HTONL(0x1c93caff),   // Block3: 9489:55ff
      PP_HTONL(0xfe9b8850)    // Block4: fe1e:8938
    );
    IP_ADDR6(
      &netmask,                // 目标地址变量
      PP_HTONL(0xffffffff),   // Block1: ffff:ffff
      PP_HTONL(0xffffffff),   // Block2: ffff:ffff
      PP_HTONL(0x00000000),   // Block3: 0000:0000
      PP_HTONL(0x00000000)    // Block4: 0000:0000
    );
    IP_ADDR6(
      &gw,                // 目标地址变量
      PP_HTONL(0xfe800000),   // Block1: fe80:: 
      PP_HTONL(0x00000000),   // Block2: 0000:0000
      PP_HTONL(0x1c93caff),   // Block3: 9489:55ff
      PP_HTONL(0xfe9b8852)    // Block4: fe1e:8939
    );
  //ret = start_networking(&net_interface, &ipaddr, &netmask, &gw, mac_ethernet_address);
  //uint8_t mac[6] = {0x00, 0x0A, 0x35, 0x00, 0x22, 0x01};

    ret = start_networking(
        &net_interface,
        &ipaddr,
        &netmask,
        &gw,
        mac_ethernet_address
      );
    if ( ret != 0 ) {
        return;
      }

// 在Init函数添加
net_interface.ip6_autoconfig_enabled = 1;
nd6_tmr(NULL); // 启动邻居发现定时器
    if (ret != 0) {
        return;
    }

    rtems_shell_init_environment();
    test();
/* 
    sc = rtems_shell_init(
        "SHLL",
        RTEMS_MINIMUM_STACK_SIZE * 4,
        100,
        "/dev/console",
        false,
        true,
        NULL);
//    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  */  
    //TEST_END();
    rtems_test_exit(0);
}

// 在配置文件添加
#define LWIP_IPV6_DHCP6            1
#define LWIP_DHCP6_GET_NTP         0

#define LWIP_IPV6                   1
#define LWIP_ICMP6                  1

//opt.h
//启用IPv6地址作用域支持，确保例如链路本地地址确实是作为链路本地地址处理的
#define LWIP_IPV6_SCOPES           1
//在运行时执行检查以验证地址在需要的地方是否正确设置了区域
#define LWIP_IPV6_SCOPES_DEBUG     1
//每个网络接口的IPv6地址数量
#define LWIP_IPV6_NUM_ADDRESSES    3
//在网络接口之间转发IPv6数据包
#define LWIP_IPV6_FORWARD          1
#define LWIP_IPV6_SEND_ROUTER_SOLICIT 1
#define LWIP_IPV6_ADDRESS_LIFETIMES  1
#define LWIP_DHCP6_GETS_NTP         1
#define LWIP_IPV6_MLD               1
#define LWIP_DHCP    0

//LWIP_IPV6_FRAG==1: 对太大的传出IPv6数据包进行分片
///LWIP_IPV6_REASS==1: 重新组装到达的已分片IPv6数据包
//LWIP_IPV6_SEND_ROUTER_SOLICIT==1: 在网络启动期间发送路由器请求消息。
//LWIP_IPV6_AUTOCONFIG==1: 启用无状态地址自动配置，遵循RFC 4862。
///LWIP_IPV6_ADDRESS_LIFETIMES==1: 为每个IPv6地址保持有效和首选生命周期。
//LWIP_IPV6_DUP_DETECT_ATTEMPTS=[0..7]: 重复地址检测尝试的次数。
//LWIP_ICMP6==1: 启用ICMPv6（根据RFC是必需的）
//LWIP_ICMP6_DATASIZE: 在ICMPv6错误消息中发送的原始数据包的字节数（0 = IP6_MIN_MTU_LENGTH的默认值）
//LWIP_ICMP6_HL: ICMPv6消息的默认跳数限制
/**
 * @defgroup lwip_opts_mld6 多播听众发现
 * @ingroup lwip_opts_ipv6
 * @{
 */
//LWIP_IPV6_MLD==1: 启用多播听众发现协议
//MEMP_NUM_MLD6_GROUP: 可以加入的IPv6多播组的最大数量
/**
 * @defgroup lwip_opts_nd6 邻居发现
 * @ingroup lwip_opts_ipv6
 * @{
 */
//LWIP_ND6_QUEUEING==1: 在解析MAC地址时队列传出IPv6数据包 
//MEMP_NUM_ND6_QUEUE: 在MAC解析期间队列的最大IPv6数据包数量。 
//LWIP_ND6_NUM_NEIGHBORS: IPv6邻居缓存中的条目数
//LWIP_ND6_NUM_DESTINATIONS: IPv6目标缓存中的条目数
//LWIP_ND6_NUM_PREFIXES: IPv6链路本地前缀缓存中的条目数 
// LWIP_ND6_NUM_ROUTERS: IPv6默认路由器缓存中的条目数
//LWIP_ND6_MAX_MULTICAST_SOLICIT: 发送的最大多播请求消息数（邻居请求和路由器请求）
//LWIP_ND6_MAX_UNICAST_SOLICIT: 在邻居可达性检测期间发送的最大单播邻居请求消息数
//LWIP_ND6_REACHABLE_TIME: 默认邻居可达时间（以毫秒计）
//LWIP_ND6_RETRANS_TIMER: 默认的重新传输计时器，用于请求消息
//LWIP_ND6_DELAY_FIRST_PROBE_TIME: 在邻居可达性检测期间，在发送第一个单播邻居请求消息之前的延迟时间。
//LWIP_ND6_ALLOW_RA_UPDATES==1: 允许路由器公告消息更新可达时间、重新传输计时器和网络接口MTU。
//LWIP_ND6_TCP_REACHABILITY_HINTS==1: 允许TCP为连接的目标提供邻居发现可达性提示。
//LWIP_ND6_RDNSS_MAX_DNS_SERVERS > 0: 使用IPv6路由器公告递归DNS服务器选项（如RFC 6106所述）
/**
 * @defgroup lwip_opts_dhcpv6 DHCPv6
 * @ingroup lwip_opts_ipv6
 * @{
 */
//LWIP_IPV6_DHCP6==1: 启用DHCPv6有状态/无状态地址自动配置
//LWIP_IPV6_DHCP6_STATEFUL==1: 启用DHCPv6有状态地址自动配置。（尚不支持！）
//LWIP_IPV6_DHCP6_STATELESS==1: 启用DHCPv6无状态地址自动配置。
//LWIP_DHCP6_GETS_NTP==1: 通过DHCPv6请求NTP服务器。对于每个响应包，都会调用一个回调函数，该函数由端口提供：
//LWIP_DHCP6_MAX_NTP_SERVERS 请求的最大NTP服务器数量
//LWIP_DHCP6_MAX_DNS_SERVERS > 0: 通过DHCPv6请求DNS服务器。






#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32
#define CONFIGURE_NETWORKING


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



















