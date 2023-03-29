/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <hi_io.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio_ex.h"
#include "iot_gpio.h"
#include "iot_adc.h"
#include "hi_adc.h"
#include "iot_watchdog.h"
#include "iot_errno.h"

#define DEBUG 0

#define LED_TEST_GPIO IOT_IO_NAME_GPIO_9
#define PD_TEST_GPIO IOT_IO_NAME_GPIO_12
#define PD_ADC_CHANNEL HI_ADC_CHANNEL_0

#define MES_LEN 18
#define VALID_MES_LEN 6

#define VLT_LOW 0.500  // 低电平阈值
#define VLT_HIGH 1.200  // 高电平阈值


#define UP 1
#define DOWN 0

hi_u16 reciveData[MES_LEN] = {0}; // 获取接收到的数据
hi_s8 decodeData[VALID_MES_LEN]={0};

int lowCount=0;             // 低电平计数
int highCount=0;            // 高电平计数
unsigned int dataCounter=0; // 写入数据计数器


void decode(const hi_u16 *encode, hi_u8*data)
{
    int j=4;
    for(int i=0;i<VALID_MES_LEN;i++)
    {
        if(encode[j]==0)
            data[i]='1';
        else
            data[i]='0';
        j+=2;
    }
}

//Initial GPIO used by LED and GPIO used by PD.
void gpioInit()
{
    IoTGpioInit(LED_TEST_GPIO);
    IoTGpioInit(PD_TEST_GPIO);

    IoSetFunc(LED_TEST_GPIO, IOT_IO_FUNC_GPIO_9_GPIO);
    IoSetFunc(PD_TEST_GPIO, IOT_IO_FUNC_GPIO_12_GPIO);

    IoTGpioSetDir(LED_TEST_GPIO, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(LED_TEST_GPIO, IOT_GPIO_DIR_IN);

    hi_io_set_pull(PD_TEST_GPIO, HI_IO_PULL_DOWN);
}

//Get Voltage from PD receiver.
hi_float getVlt()
{
    hi_u32 data;
    int ret = hi_adc_read((hi_adc_channel_index)PD_ADC_CHANNEL, &data, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0);
    if (ret != HI_ERR_SUCCESS)
    {
        printf("ADC Read Fail!\n");
    }
    hi_float vlt;
    vlt = hi_adc_convert_to_voltage(data);
    return vlt;
}


/* Convert Voltage to data of binary form.
*  Data is like: [0101](the header)+[valid data for 6 bits]+[10](the tail)
*/
void VlcTask()
{
    hi_float vlt;

    int dir = UP; // 写入控制

    highCount = 0;
    lowCount = 0;
    dataCounter = 0;

    int lowThres = 0;      // “0”持续采样计数
    int highThres = 0;     // “1”持续采样计数
    hi_bool highFlag = HI_FALSE; // 标记信息开始
    hi_bool lowFlag = HI_FALSE;  // 标记LowThresHold划分完毕
    hi_bool twoFlag = HI_FALSE;  // 标记HighThresHold划分完毕

    while (1)
    {
        vlt = getVlt();

        // 小于阈值
        if (vlt <= VLT_LOW && vlt>0.0)
        {
            lowCount++;
            // 下降沿波形
            if (highCount != 0)
            {
                dir = DOWN;
            }
        }

        // 高于阈值
        else if (vlt >= VLT_HIGH)
        {
            highCount++;
            // 上升沿
            if (lowCount != 0)
            {
                dir = UP;
            }
        }

        if (lowCount != 0 && highCount != 0 && dataCounter < MES_LEN)
        {
            #if DEBUG    
            printf("L%d,H%d, D%d\r\n", lowCount, highCount, dataCounter);
            #endif
            if (dir == UP)
            {
                // 信息开始
                if (highFlag == HI_FALSE && lowFlag == HI_FALSE)
                {
                    reciveData[dataCounter] = 0;
                    dataCounter++;
                    lowCount = 0;
                    twoFlag = HI_TRUE; // 启动HighThres划分
                }

                // 启动LowThres划分
                else if (highFlag == HI_TRUE && lowFlag == HI_FALSE)
                {
                    reciveData[dataCounter] = 0;
                    dataCounter++;
                    lowThres = lowCount;
                    lowCount = 0;
                    lowFlag = HI_TRUE;
                    twoFlag = HI_FALSE;
                }
                else if (highFlag && lowFlag && !twoFlag)
                {
                    // 如果差值小于8，填充一位数据
                    if (abs(lowCount - lowThres) <= 8 || lowCount<lowThres)
                    {
                        reciveData[dataCounter] = 0;
                        dataCounter++;
                        lowCount = 0;
                    }
                    else
                    {
                        reciveData[dataCounter] = 0;
                        dataCounter++;
                        if (dataCounter < MES_LEN)
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
                if (twoFlag == HI_TRUE)
                {
                    reciveData[dataCounter] = 1;
                    dataCounter++;
                    highThres = highCount;
                    highCount = 0;
                    highFlag = HI_TRUE;
                }
                else if (highFlag && lowFlag && !twoFlag)
                {
                    if (abs(highCount - highThres) < 10)
                    {
                        reciveData[dataCounter] = 1;
                        dataCounter++;
                        highCount = 0;
                    }
                    else if(abs(highCount-2*highThres)<=70)
                    {
                        reciveData[dataCounter] = 1;
                        dataCounter++;
                        if (dataCounter < MES_LEN)
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

        if (dataCounter >= MES_LEN)
        {

            #if DEBUG
            for (int i = 0; i < MES_LEN; i++)
            {
                printf("%d ", reciveData[i]);
            }
            printf("\r\n");
            #endif

            decode(reciveData,decodeData);

            #if DEBUG
            for(int i=0;i<VALID_MES_LEN;i++)
            printf("%c",decodeData[i]);
            printf("\n");
            #endif

            if(strcmp(decodeData,"000000")==0)
            IoTGpioSetOutputVal(LED_TEST_GPIO,IOT_GPIO_VALUE0);
            else if(strcmp(decodeData,"000001")==0)
            IoTGpioSetOutputVal(LED_TEST_GPIO,IOT_GPIO_VALUE1);


            memset_s(reciveData, sizeof(reciveData), 0x0, sizeof(reciveData));
            lowCount = 0;
            highCount = 0;
            dataCounter = 0;
            lowThres = 0;
            highThres = 0;
            lowFlag = HI_FALSE;
            highFlag = HI_FALSE;
            twoFlag = HI_FALSE;
        }
    }
}

#if DEBUG
void testTask()
{
    hi_float vlt;
    while(1) 
    {
    vlt =getVlt();
    // printf("%.3f\n\r",vlt);

    if(vlt<=VLT_LOW)
    lowCount++;

    if(vlt>=VLT_HIGH)
    {
        printf("%d\n",lowCount);
        lowCount=0;
    }
    }
}
#endif

static void VlcEntry(void)
{
    osThreadAttr_t attr;
    IoTWatchDogDisable();
    gpioInit();
    attr.name = "TestTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 8192;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)VlcTask, NULL, &attr) == NULL)
    {
        printf("[LedExample] Falied to create LedTask!\n");
    }
}

APP_FEATURE_INIT(VlcEntry);
