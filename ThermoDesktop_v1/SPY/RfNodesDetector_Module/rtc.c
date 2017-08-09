#include "rtc.h"

volatile uint32 nTick = 0;

void vRTCInit()
{
  // TIM4 for RTC
  TIM4_ARR = 250;
  
  TIM4_PSCR_PSC = TIM_PRESCALER_128; // FOSC/128

  TIM4_IER_UIE = 1; // enable interrupt

  TIM4_CR1 = 0;
  TIM4_CR1_ARPE = 1; // auto-reload enable
  TIM4_CR1_CEN  = 1; // enable timer
}

uint32 nGetTickCount()
{
  uint32 res;
  TIM4_IER_UIE = 0; // disable timer interrupt
  res = nTick;
  TIM4_IER_UIE = 1; // enable timer interrupt
  return res;
}

#pragma vector = TIM4_OVR_UIF_vector
__interrupt void vRTC_Interrupt()
{
  nTick++;
  TIM4_SR_UIF = 0; // Clear interrupt flag
}