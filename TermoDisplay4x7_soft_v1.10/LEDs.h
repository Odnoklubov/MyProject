///////////////////////////////////////////////////////////////////////////////
//!
//! \file LEDs.h
//!
//! \brief 
//! Function and definition for control GPIO
//!
//! \author Sergey Vasilenko "sergey@mcsys.by"
//!
////////////////////////////////////////////////////////////////////////////////
#ifndef _LEDS_H_
#define _LEDS_H_

#include "iostm8s105c6.h"

////////////////////////////////////////////////////////////////////////////////
// Macro definitions
////////////////////////////////////////////////////////////////////////////////
#define LED0_ON 	        PD_ODR|=MASK_PD_ODR_ODR0
#define LED1_ON 	        PE_ODR|=MASK_PE_ODR_ODR5
#define LED2_ON 	        PC_ODR|=MASK_PC_ODR_ODR1
#define LED3_ON 	        PE_ODR|=MASK_PE_ODR_ODR7
#define LED4_ON 	        PE_ODR|=MASK_PE_ODR_ODR6

#define LED0_OFF 	        PD_ODR&=~MASK_PD_ODR_ODR0
#define LED1_OFF		PE_ODR&=~MASK_PE_ODR_ODR5
#define LED2_OFF	        PC_ODR&=~MASK_PC_ODR_ODR1
#define LED3_OFF		PE_ODR&=~MASK_PE_ODR_ODR7
#define LED4_OFF         	PE_ODR&=~MASK_PE_ODR_ODR6


////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! 
//! \brief Prepare needed port for work. All the necessary settings are made inside
//!  
////////////////////////////////////////////////////////////////////////////////
void LEDs_Init(void); 
#endif //_LEDS_H_
