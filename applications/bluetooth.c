
#include <bluetooth.h>
#include <inoutmsg.h>


#define YGP_BLUETOOTH_DEBUG

static rt_device_t serial;
#define UART_NAME           "uart2"
#define UART_BAUD_RATE      BAUD_RATE_9600
#define UART_DATA_BITS      DATA_BITS_8
#define UART_STOP_BITS      STOP_BITS_1
#define UART_PARITY         PARITY_NONE
#define UART_BUFSZ          512                     //HC-08手册上规定一个数据包不能超过500个字节

static struct rt_timer  timer1;                     //数据帧间隔判断定时器
#define TIMER1_NAME         "T0927"                 //在操作系统中不能重名
#define TIMER1_TIMING       60                      //定时器定时

static int charsize = 0;                            //串口接收到的字符个数统计
#define MSGSIZE              2048                    //512
static char receive_msg[MSGSIZE];
static char send_msg[MSGSIZE];




/* 超 时 时 回 调 的 处 理 函数 */
static void timer1_timeout(void* parameter){
    //这里想要优化的话可使用消息列队起+处理线程
    rt_device_read(serial, 0, receive_msg, charsize);
#ifdef YGP_BLUETOOTH_DEBUG
    rt_kprintf("serial read receive_msg=%s;\n",receive_msg);
#endif /* YGP_BLUETOOTH_DEBUG */

    iom1(receive_msg,MSGSIZE,send_msg,MSGSIZE);
    rt_kprintf("send_msg=%s;\n",send_msg);
    rt_device_write(serial, 0, send_msg, strlen(send_msg));

    //复位字符统计和接收缓冲区
    charsize = 0;
    memset(receive_msg,0,MSGSIZE);
}

/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    charsize++;
    //启动定时器
    rt_timer_start(&timer1);
    return RT_EOK;
}


int my_bluetooth_up(void){
    /* 查找系统中的串口设备 */
    serial = rt_device_find(UART_NAME);
    if (!serial)
    {
#ifdef YGP_BLUETOOTH_DEBUG
        rt_kprintf("find %s failed!\n", UART_NAME);
#endif /* #ifdef YGP_BLUETOOTH_DEBUG */
        return RT_ERROR;
    }
    //更改串口参数
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */
    //修改串口配置参数
    config.baud_rate = UART_BAUD_RATE;              //修改波特率为 9600
    config.data_bits = UART_DATA_BITS;              //数据位 8
    config.stop_bits = UART_STOP_BITS;              //停止位 1
    config.bufsz     = UART_BUFSZ;                  //修改缓冲区 buff size
    config.parity    = UART_PARITY;                 //无奇偶校验位
    //step3：控制串口设备。通过控制接口传入命令控制字，与控制参数
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);

    //使用静态的定时器
    rt_timer_init(&timer1, TIMER1_NAME,                 /* 定 时 器 名 字是 timer1 */
                  timer1_timeout,                       /* 超 时 时 回 调 的 处 理 函数 */
                    RT_NULL,                            /* 超 时 函 数 的 入 口 参 数 */
                    TIMER1_TIMING,                      /* 定 时 长 度， 以 OS Tick 为单 位， */ //调试期间10s一个
                    RT_TIMER_FLAG_ONE_SHOT);            /* 单 次 定 时 器 */

    /* 以中断接收及轮询发送模式打开串口设备 */
    rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(serial, uart_input);

    return RT_EOK;
}
MSH_CMD_EXPORT(my_bluetooth_up, my_bluetooth_up);


void H001(){
    rt_device_write(serial, 0, "ABCDEF", 6);
}
MSH_CMD_EXPORT(H001, H001);
