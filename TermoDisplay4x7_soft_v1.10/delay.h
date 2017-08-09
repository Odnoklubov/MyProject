#ifndef _DELAY_H_
#define _DELAY_H_

#include "iostm8s105c6.h"
#include "rtc.h"
#include "types.h"

void DelayInit(void);
void Delay_1us(char i);
void Delay_us(unsigned short i);
void Delay_ms(UINT32 i);
void Delay_msSafe(UINT32 i);

#endif