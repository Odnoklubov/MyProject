#ifndef _DELAY_H_
#define _DELAY_H_

#include "iostm8s105c6.h"
#include "types.h"
#include "rtc.h"
void DelayInit(void);
void Delay_1us(char i);
void Delay_us(unsigned short i);
void Delay_ms(uint32 i);

#endif