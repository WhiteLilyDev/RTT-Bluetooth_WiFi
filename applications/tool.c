
#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <arpa/inet.h>
#include <netdev.h>       /* 当需要网卡操作是，需要包含这两个头文件 */

#include <wifi.h>



#define ESP8266_DEIVCE_NAME     "esp0"



void getIP(char* IP){
    struct netdev *netdev = RT_NULL;
    netdev = netdev_get_by_name(ESP8266_DEIVCE_NAME);
    //sprintf(IP,"%s:%d",inet_ntoa(netdev->ip_addr),SERVER_PORT);
    sprintf(IP,"%s:%d",SERVER_IP,SERVER_PORT);
}
