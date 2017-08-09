#include "delay.h"

void DelayInit(void)
{
  TIM4_CR1 = 0;
  TIM4_PSCR = 0;
  TIM4_CNTR = 0;
}

void Delay_1us(char i)
{
  i *= 16;
  TIM4_CR1 = 0;
  TIM4_CNTR = 0xff - i;
  TIM4_CR1 = (MASK_TIM3_CR1_CEN | MASK_TIM4_CR1_OPM);
  while(TIM4_CR1 & MASK_TIM4_CR1_CEN);
}

void Delay_us(unsigned short i)
{
  i /= 5;
  TIM4_CR1_bit.OPM = 0;
  TIM4_ARR = 80;
  TIM4_CNTR = 60;
  TIM4_SR_bit.UIF = 0;
  TIM4_CR1_bit.CEN = 1;

  while(i--) 
  {
    while(0 == TIM4_SR_bit.UIF) ;
    TIM4_SR_bit.UIF = 0;
  }

  TIM4_ARR = 255;
  TIM4_CR1_bit.CEN = 0;
  TIM4_CR1_bit.OPM = 1;
}

void Delay_ms(uint32 i)
{
 uint32 timerTickAmount = nGetTickCount();
  while(timerTickAmount + ms(i) > nGetTickCount());
  //while(--i) Delay_us(1000);
}
