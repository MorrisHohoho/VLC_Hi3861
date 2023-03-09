#include<stdio.h>
#include<unistd.h>
#include<hi_types_base.h>
#include<hi_io.h>
#include <hi_adc.h>
#include<hi_stdlib.h>  
#include<hi_early_debug.h>
#include<hi_gpio.h>
#include<hi_task.h>
#include"ohos_init.h"
#include"cmsis_os2.h"
#include "iot_gpio.h"


//获取digital输入
void my_gpio_isr_demo(void *arg)
{
    (void)arg;
    hi_u32 ret;

    printf("----- gpio isr demo -----\r\n");

    (hi_void)hi_gpio_init();    //初始化GPIO
    
    hi_io_set_func(HI_IO_NAME_GPIO_11, HI_IO_FUNC_GPIO_11_GPIO); //设置GPIO11为普通GPIO引脚

    //GPIO初始化状态输出
    ret = hi_gpio_set_dir(HI_GPIO_IDX_11, HI_GPIO_DIR_IN);
    if (ret != HI_ERR_SUCCESS) {
        printf("===== ERROR ======gpio -> hi_gpio_set_dir1 ret:%d\r\n", ret);
        return;
    }

    

    while(1)
    {
        IotGpioValue io_status;     //GPIO输入值
        IoTGpioGetInputVal(11,&io_status);  //获取GPIO输入
        if(io_status != IOT_GPIO_VALUE0){
            printf("LOW \r\n");
        }
        else
        {
            printf("HIGH\r\b");
        }
        usleep(1000000);        //延迟1s
    }
    /*
    没做任何解编码处理。
    遮住PD时，获取到LOW
    不遮住PD时，获取到HIGH
    */
    }


//进程
void adc_demo(void)
{
    osThreadAttr_t attr;
    
    attr.name = "KeyTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 2048;
    attr.priority = 26;

    if (osThreadNew((osThreadFunc_t)my_gpio_isr_demo, NULL, &attr) == NULL) {
        printf("[adc_demo] Falied to create KeyTask!\n");
    }
    
}


SYS_RUN(adc_demo);