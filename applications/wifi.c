#include <inoutmsg.h>

#include <wifi.h>
#include <arpa/inet.h>
#include <netdev.h>       /* 当需要网卡操作是，需要包含这两个头文件 */

#include <rtthread.h>
#include <sys/socket.h> /* 使用BSD socket，需要包含socket.h头文件 */
#include <netdb.h>
#include <string.h>
#include <finsh.h>
#define BUFSZ   1024
char send_data[BUFSZ] = {0}; /* 发送用到的数据 */
char recv_data[BUFSZ] = {0};

struct rt_semaphore sem1;       //用于sockt连接重启线程
void tcpclient()
{
    //等待网卡存在IP
    struct netdev *netdev = RT_NULL;
    netdev = netdev_get_by_name("esp0");
    while(1){
        if (netdev->ip_addr.addr>0) {
            break;
        }else {
            rt_thread_mdelay(2000);
        }
    }
    //rt_thread_mdelay(2000);
    rt_kprintf("Start connecting to WiFi server...\n");

    int ret;
    struct hostent *host;
    int sock, bytes_received;
    struct sockaddr_in server_addr;
    const char *url;
    int port;

    url = SERVER_IP;
    port = SERVER_PORT;
    /* 通过函数入口参数url获得host地址（如果是域名，会做域名解析） */
    host = gethostbyname(url);

    /* 创建一个socket，类型是SOCKET_STREAM，TCP类型 */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        /* 创建socket失败 */
        rt_kprintf("Socket error\n");
        goto __end;
        return;
    }
    /* 初始化预连接的服务端地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    /* 连接到服务端 */
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        /* 连接失败 */
        rt_kprintf("WIFI server Connect fail!\n");
        closesocket(sock);
        goto __end;
        return;
    }
    rt_kprintf("WIFI server Connect success!\n");
    while (1)
    {
        /* 从sock连接中接收最大BUFSZ - 1字节数据 */
        bytes_received = recv(sock, recv_data, BUFSZ - 1, 0);
        if (bytes_received < 0)
        {
            /* 接收失败，关闭这个连接 */
            closesocket(sock);
            rt_kprintf("\nreceived error,close the socket.\r\n");

            break;
        }
        else if (bytes_received == 0)
        {
            /* 默认 recv 为阻塞模式，此时收到0认为连接出错，关闭这个连接 */
            closesocket(sock);
            rt_kprintf("\nreceived error,close the socket.\r\n");
            break;
        }
        /* 有接收到数据，把末端清零 */
        recv_data[bytes_received] = '\0';
        /* 在控制终端显示收到的数据 */
        rt_kprintf("\nReceived data =%s;\n", recv_data);
        iom1(recv_data,BUFSZ,send_data,BUFSZ);
        /* 发送数据到sock连接 */
        ret = send(sock, send_data, strlen(send_data), 0);
        if (ret < 0)
        {
            /* 接收失败，关闭这个连接 */
            closesocket(sock);
            rt_kprintf("\nsend error,close the socket.\r\n");
            break;
        }
        else if (ret == 0)
        {
            /* 打印send函数返回值为0的警告信息 */
            rt_kprintf("\n Send warning,send function return 0.\r\n");
        }
    }
    goto __end;
    __end:
    rt_hw_cpu_reset();
    rt_sem_release(&sem1);

    return;
}
void wific2();
rt_thread_t twific = RT_NULL;
void my_wifi_up(void){
    rt_sem_init(&sem1,"sem1",0,RT_IPC_FLAG_FIFO);

    rt_thread_mdelay(2000);

    twific = rt_thread_create("wific", tcpclient, RT_NULL, 2048, 10, 20);
    if (twific != RT_NULL)
    {
        rt_thread_startup(twific);
        rt_kprintf("tcpclient up!\n");
    }else {
        rt_kprintf("tcpclient ERROR!\n");
    }

    rt_thread_t twific2 = RT_NULL;

    twific2 = rt_thread_create("wific2", wific2, RT_NULL, 1024, 10, 20);
    if (twific2 != RT_NULL)
    {
        rt_thread_startup(twific2);
        rt_kprintf("wific2 up!\n");
    }else {
        rt_kprintf("wific2 ERROR!\n");
    }
}

int errorcount=0;
void wific2(){
    while(1){
        rt_sem_take(&sem1, RT_WAITING_FOREVER);
        errorcount++;
        if (errorcount==5) {
            rt_hw_cpu_reset();
        }
        //删除线程
        rt_thread_delete(twific);

        //重启前程
        twific = rt_thread_create("wific", tcpclient, RT_NULL, 2048, 10, 20);
        if (twific != RT_NULL)
        {
            rt_thread_startup(twific);
            rt_kprintf("tcpclient up!\n");
        }else {
            rt_kprintf("tcpclient ERROR!\n");
        }

        rt_thread_mdelay(2000);
    }
}


