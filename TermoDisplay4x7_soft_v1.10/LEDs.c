#include "LEDs.h"

void LEDs_Init() 
{
    //Setting LED1 pin as output on PortC pin 1
    PC_ODR&=~MASK_PC_ODR_ODR1;   // output to 0 
    PC_CR1|=MASK_PC_CR1_C11;     // push-pull output
    PC_CR2&=~MASK_PC_CR2_C21;    // low speed output
    PC_DDR|=MASK_PC_DDR_DDR1;    // as output
    //Setting LED2 pin as output on PortE pin 5
    PE_ODR&=~MASK_PE_ODR_ODR5;   // output to 0 
    PE_CR1|=MASK_PE_CR1_C15;     // push-pull output
    PE_CR2&=~MASK_PE_CR2_C25;    // low speed output
    PE_DDR|=MASK_PE_DDR_DDR5;    // as output
    //Setting LED3 pin as output on PortE pin 6
    PE_ODR&=~MASK_PE_ODR_ODR6;   // output to 0 
    PE_CR1|=MASK_PE_CR1_C16;     // push-pull output
    PE_CR2&=~MASK_PE_CR2_C26;    // low speed output
    PE_DDR|=MASK_PE_DDR_DDR6;    // as output
    //Setting LED4 pin as output on PortE pin 7
    PE_ODR&=~MASK_PE_ODR_ODR7;   // output to 0 
    PE_CR1|=MASK_PE_CR1_C17;     // push-pull output
    PE_CR2&=~MASK_PE_CR2_C27;    // low speed output
    PE_DDR|=MASK_PE_DDR_DDR7;    // as output 
}