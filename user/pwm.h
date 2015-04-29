#ifndef ESP_PWM_H
#define ESP_PWM_H

#define PWM_DEPTH 0xFF
#define PWM_DEPTH_BIT 8
//#define PWM_FACTOR 0x10
//int32_t PWM_FACTOR = 0x10;
#include "platform_config.h"

void ICACHE_RAM_ATTR pwm_set(int pin, uint8_t value);

#endif
