/*
解决办法：利用数点，统计对应的信息
为什么一个系统内核没有计时器啊？

0101：包头，同时用作信息划分
*/


//Not gonna work.
#include <stdio.h>
#include <unistd.h>
#include <hi_types_base.h>
#include <hi_io.h>
#include <hi_adc.h>
#include <hi_stdlib.h>
#include <hi_early_debug.h>
#include <hi_gpio.h>
#include <hi_task.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"

#define VLT_LOW 0.205  // 低电平阈值
#define VLT_HIGH 1.145 // 高电平阈值
#define UP 1
#define DOWN 0

hi_u16 reciveData[12] = {0}; // 获取接收到的数据
hi_u16 deData[4]={0};        //解码后的数据

int lowCount;             // 低电平计数
int highCount;            // 高电平计数
unsigned int dataCounter; // 写入数据计数器

// 信息处理
void my_gpio_isr_demo(void *arg)
{
    (void)arg;
    hi_u32 ret;

    unsigned int data;
    hi_float vlt;
    int dir = UP; // 写入控制

    highCount = 0;
    lowCount = 0;
    dataCounter = 0;

    int lowThres = 0;      // “0”持续采样计数
    int highThres = 0;     // “1”持续采样计数
    bool highFlag = false; // 标记信息开始
    bool lowFlag = false;  // 标记LowThresHold划分完毕
    bool twoFlag = false;  // 标记HighThresHold划分完毕

    printf("----- gpio isr demo -----\r\n");


    //使用GPIO12 作为输入
    //GPIO12=机器人板上的传感器2的IN
    (hi_void) hi_gpio_init();
    hi_io_set_func(HI_IO_NAME_GPIO_12, HI_IO_FUNC_GPIO_12_GPIO);
    ret = hi_gpio_set_dir(HI_GPIO_IDX_12, HI_GPIO_DIR_IN);
    if (ret != HI_ERR_SUCCESS)
    {
        printf("===== ERROR ======gpio -> hi_gpio_set_dir1 ret:%d\r\n", ret);
        return;
    }

    ret = hi_gpio_set_dir(HI_GPIO_IDX_11,HI_GPIO_DIR_OUT);
    if (ret != HI_ERR_SUCCESS)
    {
        printf("===== ERROR ======gpio -> hi_gpio_set_dir1 ret:%d\r\n", ret);
        return;
    }

    hi_io_set_pull(HI_IO_NAME_GPIO_12, HI_IO_PULL_DOWN); // 默认为下拉状态

    while (1)
    {

        // 读取ADC值，并转换为电压值
        ret = hi_adc_read((hi_adc_channel_index)HI_ADC_CHANNEL_0, &data, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0);
        if (ret != HI_ERR_SUCCESS)
        {
            printf("ADC Read Fail!\n");
        }
        vlt = hi_adc_convert_to_voltage(data);

        // 输出数据
        // if (vlt <= VLT_LOW || vlt >= VLT_HIGH)
        // printf("%.3f\r\n", vlt);

        // 小于阈值
        if (vlt <= VLT_LOW)
        {
            lowCount++;

            //下降沿波形
            if (highCount != 0)
            {
                dir = DOWN;
            }
        }

        //高于阈值
        if (vlt >= VLT_HIGH)
        {
            highCount++;
            
            //上升沿
            if (lowCount != 0)
            {
                dir = UP;
            }
        }

        // 发生数据跳变
        if (lowCount != 0 && highCount != 0 && dataCounter < 12)
        {
            printf("Low %d,High %d, Data %d\r\n", lowCount, highCount, dataCounter);
            if (dir == UP)
            {

                // 信息开始
                if (highFlag == false && lowFlag == false)
                {
                    reciveData[dataCounter] = 0;
                    dataCounter++;
                    lowCount = 0;
                    twoFlag = true; // 启动HighThres划分
                }

                // 启动LowThres划分
                else if (highFlag == true && lowFlag == false)
                {
                    reciveData[dataCounter] = 0;
                    dataCounter++;
                    lowThres = lowCount;
                    lowCount = 0;
                    lowFlag = true;
                    twoFlag = false;
                }
                else if (highFlag && lowFlag && !twoFlag)
                {
                    // 如果差值小于15，填充一位数据
                    if (abs(lowCount - lowThres) < 15)
                    {
                        reciveData[dataCounter] = 0;
                        dataCounter++;
                        lowCount = 0;
                    }
                    else
                    {
                        reciveData[dataCounter] = 0;
                        dataCounter++;
                        if (dataCounter < 12)
                        {
                            reciveData[dataCounter] = 0;
                            dataCounter++;
                        }
                        lowCount = 0;
                    }
                }
            }

            else if (dir == DOWN)
            {
                // HighThres划分
                if (twoFlag == true)
                {
                    reciveData[dataCounter] = 1;
                    dataCounter++;
                    highThres = highCount;
                    highCount = 0;
                    highFlag = true;
                }
                else if (highFlag && lowFlag && !twoFlag)
                {
                    if (abs(highCount - highThres) < 15)
                    {
                        reciveData[dataCounter] = 1;
                        dataCounter++;
                        highCount = 0;
                    }
                    else
                    {
                        reciveData[dataCounter] = 1;
                        dataCounter++;
                        if (dataCounter < 12)
                        {
                            reciveData[dataCounter] = 1;
                            dataCounter++;
                        }
                        highCount = 0;
                    }
                }
                else
                {
                    highCount = 0;
                }
            }
        }
        
        if (dataCounter >= 12)
        {
            for (int i = 0; i < 12; i++)
            {
                printf("%d ", reciveData[i]);
            }
            printf("\r\n");
            //Decode();
            memset_s(reciveData, sizeof(reciveData), 0x0, sizeof(reciveData));
            lowCount = 0;
            highCount = 0;
            dataCounter = 0;
            lowThres = 0;
            highThres = 0;
            lowFlag = false;
            highFlag = false;
            twoFlag = false;
        }
    }
}


// 进程
void adc_demo(void)
{
    osThreadAttr_t attr;

    attr.name = "KeyTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 8192;
    attr.priority = 26;

    if (osThreadNew((osThreadFunc_t)my_gpio_isr_demo, NULL, &attr) == NULL)
    {
        printf("[adc_demo] Falied to create KeyTask!\n");
    }
}

APP_FEATURE_INIT(adc_demo);
