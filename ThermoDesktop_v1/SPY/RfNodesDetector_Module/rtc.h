#ifndef __RTC_H_
 #define __RTC_H_

#include <stdint.h>
#include <stdio.h>
#include "iostm8s105c6.h"
#include "types.h"

#define RTC_RATE_HZ       500 // "дискретность" часов

#define ms(x)             ((uint32)x*RTC_RATE_HZ/1000)          // макрос пересчитывает мс в тики
#define tick(x)           ((uint32)x * 1000 / RTC_RATE_HZ)     // макрос пересчитывает тики в мс

void vRTCInit();          // инициализация RTC
uint32_t nGetTickCount(); // возвращает количество тиков, прощедшее после инициализации RTC

#endif