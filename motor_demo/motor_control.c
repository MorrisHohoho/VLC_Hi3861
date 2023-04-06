#include <stdio.h>
#include <unistd.h>

#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_watchdog.h"
#include "iot_pwm.h"

#define IOT_PWM_PORT_PWM0   0
#define IOT_PWM_PORT_PWM1   1
#define IOT_PWM_PORT_PWM3   3
#define IOT_PWM_PORT_PWM4   4
#define IOT_FREQ            65535
#define IOT_DUTY            99
#define LED_TEST_GPIO12 12 //LED引脚
char a[18];
char step[6];
// PWM取值：分频系数[1, 65535] PWM value: frequency division coefficient [1, 65535]
void *LedTask(const char *arg)
{
    IoTGpioInit(LED_TEST_GPIO12);
    //设置为输出
    IoTGpioSetDir(LED_TEST_GPIO12, IOT_GPIO_DIR_OUT);
    (void)arg;
    return NULL;
}
void Initialization()
{
    a[0] = '0';
    a[1] = '1';
    a[2] = '0';
    a[3] = '1';
    a[16] = '1';
    a[17] = '0';
}
void Getinfo(int i)
{
    if(step[i] == '1')
    {
        a[2*(i + 2) ] = '0';
        a[2*(i + 2) + 1] = '1';
    }
    if(step[i] == '0')
    {
        a[2*(i + 2)] = '1';
        a[2*(i + 2) + 1] = '0';
    }
}
void LeftWheelForword(void)
{
    IoTPwmStart(IOT_PWM_PORT_PWM3, IOT_DUTY, IOT_FREQ);
}

void LeftWheelBackword(void)
{
    IoTPwmStart(IOT_PWM_PORT_PWM4, IOT_DUTY, IOT_FREQ);
}

void LeftWheelStop(void)
{
    IoTPwmStop(IOT_PWM_PORT_PWM3);
    IoTPwmStop(IOT_PWM_PORT_PWM4);
}

void RightWheelForword(void)
{
    IoTPwmStart(IOT_PWM_PORT_PWM0, IOT_DUTY, IOT_FREQ);
}

void RightWheelBackword(void)
{
    IoTPwmStart(IOT_PWM_PORT_PWM1, IOT_DUTY, IOT_FREQ);
}

void RightWheelStop(void)
{
    IoTPwmStop(IOT_PWM_PORT_PWM0);
    IoTPwmStop(IOT_PWM_PORT_PWM1);
}
void Stop(void)
{
    LeftWheelStop();
    RightWheelStop();
}
void Pre_stop()
{
    step[0] = '1';
    step[1] = '0';
    step[2] = '1';
    step[3] = '0';
    step[4] = '0';
    step[5] = '0';
    for(int i = 0; i < 6; i++)
    {
        Getinfo(i);
    }

}
void GoStraight(void)
{
    Stop();
    LeftWheelForword();
    RightWheelForword();
}
void Pre_straight()
{
    step[0] = '1';
    step[1] = '0';
    step[2] = '0';
    step[3] = '0';
    step[4] = '0';
    step[5] = '1';
    for(int i = 0; i < 6; i++)
    {
        Getinfo(i);
    }
}
void TurnLeft(void)
{
    Stop();
    LeftWheelStop();
    RightWheelForword();
}
void Pre_left()
{
    step[0] = '1';
    step[1] = '0';
    step[2] = '0';
    step[3] = '0';
    step[4] = '1';
    step[5] = '0';
    for(int i = 0; i < 6; i++)
    {
        Getinfo(i);
    }
}
void TurnRight(void)
{
    Stop();
    LeftWheelForword();
    RightWheelStop();
}
void Pre_right()
{
    step[0] = '1';
    step[1] = '0';
    step[2] = '0';
    step[3] = '0';
    step[4] = '1';
    step[5] = '1';
    for(int i = 0; i < 6; i++)
    {
        Getinfo(i);
    }
}
void GoBack(void)
{
    Stop();
    LeftWheelBackword();
    RightWheelBackword();
}
void Pre_back()
{
    step[0] = '1';
    step[1] = '0';
    step[2] = '0';
    step[3] = '1';
    step[4] = '0';
    step[5] = '0';
    for(int i = 0; i < 6; i++)
    {
        Getinfo(i);
    }
}
void Print()
{
    int j;
    for(j = 0; j < 18; j++)
    {
        putchar(a[j]);
    }
    putchar('\n');
}
void GA12N20Init(void)
{
    // 左电机GPIO5,GPIO6初始化 Initialization of left motor GPIO5 and GPIO6
    IoTGpioInit(IOT_IO_NAME_GPIO_0);
    IoTGpioInit(IOT_IO_NAME_GPIO_1);
    // 右电机GPIO9, GPIO10初始化 Right motor GPIO9, GPIO10 initialization
    IoTGpioInit(IOT_IO_NAME_GPIO_9);
    IoTGpioInit(IOT_IO_NAME_GPIO_10);

    // 设置GPIO5的管脚复用关系为PWM2输出 Set the pin multiplexing relationship of GPIO5 to PWM2 output
    IoSetFunc(IOT_IO_NAME_GPIO_0, IOT_IO_FUNC_GPIO_0_PWM3_OUT);
    // 设置GPIO6的管脚复用关系为PWM3输出 Set the pin multiplexing relationship of GPIO6 to PWM3 output
    IoSetFunc(IOT_IO_NAME_GPIO_1, IOT_IO_FUNC_GPIO_1_PWM4_OUT);
    // 设置GPIO9的管脚复用关系为PWM0输出 Set the pin multiplexing relationship of GPIO9 to PWM0 output
    IoSetFunc(IOT_IO_NAME_GPIO_9, IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    // 设置GPIO10的管脚复用关系为PWM01输出 Set the pin multiplexing relationship of GPIO10 to PWM01 output
    IoSetFunc(IOT_IO_NAME_GPIO_10, IOT_IO_FUNC_GPIO_10_PWM1_OUT);

    // GPIO5方向设置为输出 GPIO5 direction set to output
    IoTGpioSetDir(IOT_IO_NAME_GPIO_0, IOT_GPIO_DIR_OUT);
    // GPIO6方向设置为输出 GPIO6 direction set to output
    IoTGpioSetDir(IOT_IO_NAME_GPIO_1, IOT_GPIO_DIR_OUT);
    // GPIO9方向设置为输出 GPIO9 direction set to output
    IoTGpioSetDir(IOT_IO_NAME_GPIO_9, IOT_GPIO_DIR_OUT);
    // GPIO10方向设置为输出 GPIO10 direction set to output
    IoTGpioSetDir(IOT_IO_NAME_GPIO_10, IOT_GPIO_DIR_OUT);
    // 初始化PWM2 Initialize PWM2
    IoTPwmInit(IOT_PWM_PORT_PWM3);
    // 初始化PWM3 Initialize PWM3
    IoTPwmInit(IOT_PWM_PORT_PWM4);
    // 初始化PWM0 Initialize PWM0
    IoTPwmInit(IOT_PWM_PORT_PWM0);
    // 初始化PWM1 Initialize PWM1
    IoTPwmInit(IOT_PWM_PORT_PWM1);
    // 先使两个电机处于停止状态 motors stop
    RightWheelStop();
    LeftWheelStop();
}
void Light()
{
    for(int i = 0; i < 18; i++)
    {
        if(a[i] == '1')
        {
            IoTGpioSetDir(LED_TEST_GPIO12, 1);
            usleep(5000);
        }
        else if(a[i] == '0')
        {
            IoTGpioSetDir(LED_TEST_GPIO12, 0);
            usleep(5000);
        }
    }
}
void GA12N205Task(void)
{
    // 初始化电机模块 Initialize the motor module
    GA12N20Init();
    Initialization();
    while(1)
    {
        printf("Go straight for 2 seconds(100001)\n");
        Pre_straight();
        Print();
        Light();
        GoStraight();
        sleep(2);
        break;
    }
    while(1)
    {
        printf("Turn left for 2 seconds(100010)\n");
        Pre_left();
        Print();
        Light();
        TurnLeft();
        sleep(2);
        break;
    }
    while(1)
    {
        printf("Turn right for 2 seconds(100011)\n");
        Pre_right();
        Print();
        Light();
        TurnRight();
        sleep(2);
        break;
    }
    while(1)
    {
        printf("Go back for 2 seconds(100100)\n");
        Pre_back();
        Print();
        Light();
        GoBack();
       sleep(2);
        break;
    }
    while(1)
    {
        printf("Stop for 2 seconds(101000)\n");
        Pre_stop();
        Print();
        Light();
        Stop();
        sleep(2);
        break;
    }
}

void GA12N20SampleEntry(void)
{
    osThreadAttr_t attr;
    IoTWatchDogDisable();
    attr.name = "GA12N205Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 5; // 堆栈大小为1024*5,stack size 1024*5
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)GA12N205Task, NULL, &attr) == NULL) {
        printf("[GA12N205Task] Failed to create Hcsr04SampleTask!\n");
    }
}

APP_FEATURE_INIT(GA12N20SampleEntry);