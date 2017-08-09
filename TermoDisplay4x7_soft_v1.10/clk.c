#include "clk.h"

void ClkInit(void)
{
 
    CLK_SWCR |= MASK_CLK_SWCR_SWEN;	                            // Enables the switching mechanism
    CLK_SWR = MASK_CLK_SWR_HSE;		                            // Selects the high speed external as clock source
    while(!(CLK_SWCR & MASK_CLK_SWCR_SWIF))	                    // Wait until the new master clock is switching
    {
//        LED_ON; 
//        Delay_ms(10);
//        LED_OFF;
//        Delay_ms(10);
    }
  
    CLK_SWCR &= ~(MASK_CLK_SWCR_SWEN | MASK_CLK_SWCR_SWIF);         // Reset the SWIF flag and disabled switching mechanism 
    CLK_PCKENR1 =  (MASK_CLK_PCKENR1_TIM3 | MASK_CLK_PCKENR1_TIM4 | // Disables TIM1, TIM2, I2C
                    MASK_CLK_PCKENR1_UART2 | MASK_CLK_PCKENR1_SPI); // Enables SPI, UART, TIM3, TIM4
    CLK_PCKENR2 = INIT_ZERO;                                        // Disables CAN bus, ADC, AWU
    CLK_CKDIVR = INIT_ZERO;                                         // Ensure the clocks are running at full speed.
    while (CLK_SWCR & MASK_CLK_SWCR_SWBSY);                         // Pause while the clock switch is busy.
}
