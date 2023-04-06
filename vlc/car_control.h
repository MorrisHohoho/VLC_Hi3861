#ifndef __APP_DEMO_ROBOT_CAR_H__
#define __APP_DEMO_ROBOT_CAR_H__

#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <memory.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"
#include "hi_time.h"
#include "iot_pwm.h"
#include "hi_pwm.h"

#define STOP "000000"
#define FORWARD "000111"
#define LEFT   "000011"
#define RIGHT  "000110"
#define BACKWARD "111000"


void car_backward();
void car_forward();
void car_left();
void car_right();
void car_stop();

hi_bool ControlCar(const char* cmd);

#endif