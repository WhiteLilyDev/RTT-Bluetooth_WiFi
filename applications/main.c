
#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>
#include <inoutmsg.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/* PLEASE DEFINE the LED0 pin for your board, such as: PA5 */
#define LED0_PIN    GET_PIN(A, 5)

int main(void)
{
    rt_pin_mode(DOPIN1, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(DOPIN2, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(DOPIN3, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(DOPIN4, PIN_MODE_OUTPUT_OD);
    rt_pin_write(DOPIN1, 1);
    rt_pin_write(DOPIN2, 1);
    rt_pin_write(DOPIN3, 1);
    rt_pin_write(DOPIN4, 1);

    int count = 1;
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);

    extern int my_bluetooth_up(void);
    my_bluetooth_up();

    extern void my_wifi_up(void);
    my_wifi_up();

    while (count++)
    {
        /* set LED0 pin level to high or low */
        rt_pin_write(LED0_PIN, count % 2);
        //LOG_D("Hello RT-Thread!");
        rt_thread_mdelay(1000);
    }

    return RT_EOK;
}
