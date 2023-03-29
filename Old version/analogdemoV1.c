//定时器法V1
//时钟速率不够，没有找到能够记录一段时间的API
//该方法作罢

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

#define ADC_TEST_LENGTH 64
#define VLT_MIN 100
#define VLT_LOW 0.205
#define VLT_HIGH 1.145

hi_u16 g_adc_buf[ADC_TEST_LENGTH] = {0};
hi_u16 reciveData[12] = {0}; // 获取接收到的数据

uint32_t exec1;
int lowCount;            // 低电平计数
int highCount;           // 高电平计数
unsigned int writeWhich; // 写入数据控制
unsigned int dataCounter;

/*定时器回调函数*/
void Timer1_Callback(void *arg)
{
    (void)arg;

    if (writeWhich == 0)
        lowCount++;

    else if (writeWhich == 1)
        highCount++;

    printf("Low Count is %d,High Count is %d, DataCounter is %d\r\n", lowCount, highCount, dataCounter);
}

// 获取digital输入
void my_gpio_isr_demo(void *arg)
{
    (void)arg;
    hi_u32 ret;

    unsigned int data;
    hi_float vlt;

    highCount = 0;
    lowCount = 0;
    dataCounter = 0;

    printf("----- gpio isr demo -----\r\n");

    (hi_void) hi_gpio_init();
    hi_io_set_func(HI_IO_NAME_GPIO_12, HI_IO_FUNC_GPIO_12_GPIO);
    ret = hi_gpio_set_dir(HI_GPIO_IDX_12, HI_GPIO_DIR_IN);
    if (ret != HI_ERR_SUCCESS)
    {
        printf("===== ERROR ======gpio -> hi_gpio_set_dir1 ret:%d\r\n", ret);
        return;
    }

    hi_io_set_pull(HI_IO_NAME_GPIO_12, HI_IO_PULL_DOWN); // 默认为下拉状态

    osTimerId_t id1;
    uint32_t timerDelay;
    osStatus_t status;
    exec1 = 1U;
    id1 = osTimerNew(Timer1_Callback, osTimerOnce, &exec1, NULL);
    if (id1 != NULL)
    {
        // 1 tick = 1 ms;
        timerDelay = 1U;
    }

    while (1)
    {
        ret = hi_adc_read((hi_adc_channel_index)HI_ADC_CHANNEL_0, &data, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0);
        if (ret != HI_ERR_SUCCESS)
        {
            printf("ADC Read Fail!\n");
        }
        vlt = hi_adc_convert_to_voltage(data);
        if (vlt <= VLT_LOW || vlt >= VLT_HIGH)
            // printf("%.3f\r\n", vlt);

            // 小于阈值
            if (vlt <= VLT_LOW)
            {
                // 开启计时
                if (osTimerIsRunning(id1) == false)
                {
                    status = osTimerStart(id1, 1);
                    if (status != osOK)
                    {
                        printf("Timer Start is Failed!\r\n");
                    }
                    else
                        writeWhich = 0;
                }
            }
        if (vlt >= VLT_HIGH)
        {
            // 开启计时
            if (osTimerIsRunning(id1) == false)
            {
                status = osTimerStart(id1, 1);
                if (status != osOK)
                {
                    printf("Timer Start is Failed!\r\n");
                }
                else
                    writeWhich = 1;
            }
        }
        // 发生数据跳变
        if (lowCount != 0 && highCount != 0 && dataCounter < 12)
        {
            // 写入数据
            if (lowCount >= 1 && lowCount <= 3)
            {
                reciveData[dataCounter] = 0;
                dataCounter++;
                lowCount = 0;
            }
            else if (lowCount >= 4 && lowCount <= 5)
            {
                reciveData[dataCounter] = 0;
                dataCounter++;
                reciveData[dataCounter] = 0;
                dataCounter++;
                lowCount = 0;
            }
            // 长时间低电平，数据结束
            else if (lowCount > 5)
            {
                reciveData[dataCounter] = 0;
                dataCounter++;
                lowCount = 0;
            }

            // 写入数据
            if (highCount >= 1 && highCount <= 3)
            {
                reciveData[dataCounter] = 1;
                dataCounter++;
                highCount = 0;
            }
            else if (highCount >= 4 && highCount <= 5)
            {
                reciveData[dataCounter] = 1;
                dataCounter++;
                reciveData[dataCounter] = 1;
                dataCounter++;
                highCount = 0;
            }
            // 长时间高电平，数据结束
            else if (highCount > 5)
            {
                highCount = 0;
            }
        }
        if (dataCounter >= 12)
        {
            for (int i = 0; i < 12; i++)
            {
                printf("%d ", reciveData[i]);
            }
            printf("\n");
            memset_s(reciveData, sizeof(g_adc_buf), 0x0, sizeof(reciveData));
            lowCount = 0;
            highCount = 0;
            dataCounter = 0;
        }
        usleep(900);
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
    attr.stack_size = 4096;
    attr.priority = 26;

    if (osThreadNew((osThreadFunc_t)my_gpio_isr_demo, NULL, &attr) == NULL)
    {
        printf("[adc_demo] Falied to create KeyTask!\n");
    }
}

SYS_RUN(adc_demo);
