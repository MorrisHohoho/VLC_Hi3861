#include<unistd.h>
#include"stdio.h"
#include"ohos_init.h"
#include"cmsis_os2.h"
#include"iot_gpio.h"


#define LED_TEST_GPIO 9 //LED引脚

int data[12]={0,1,0,1,1,0,0,1,0,1,0,1};


void *LedTask(const char *arg)
{
//初始化 GPIO
IoTGpioInit(LED_TEST_GPIO);
//设置为输出
IoTGpioSetDir(LED_TEST_GPIO, IOT_GPIO_DIR_OUT);
(void)arg;
while (1)
{

    for(int i=0;i<12;i++)
    {
        if(data[i]==1)
        IoTGpioSetDir(LED_TEST_GPIO,1);
        else
        IoTGpioSetDir(LED_TEST_GPIO,0);
        usleep(1000);
    }
    usleep(2000000);
}
return NULL;
}

//线程设置
void led_demo(void)
{
    osThreadAttr_t attr;    //线程属性结构体
    attr.name=  "LedTask";  //线程名称
    attr.attr_bits= 0u;     //线程属性位
    attr.cb_mem = NULL;     //控制块指针
    attr.cb_size = 0U;      //控制块大小,单位:字节
    attr.stack_mem= NULL;   //线程栈指针
    attr.stack_size = 512;  //线程栈大小
    attr.priority = 26;     //线程优先级

    if(osThreadNew((osThreadFunc_t)LedTask,NULL,&attr)==NULL)
    {
        printf("[LedExample] Falied to create LedTask!\n");
    }

    /* 
    定义 : osThreadId_t osThreadNew (osThreadFunc_t func, void * argument, const osThreadAttr_t * attr)

    func : 线程回调入口函数
    argument : 传递给线程回调函数的参数
    attr : 线程属性

    功能 : 创建一个活跃的线程，当创建的线程函数的优先级高于当前的运行线程时创建的线程函数立即启动，成为新的运行线程。

    返回 : 1.成功, 返回创建的线程ID。 
           2.NULL，表示执行失败或在中断中调用该函数。
    */
     
}

//启动线程
SYS_RUN(led_demo);