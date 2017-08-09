///////////////////////////////////////////////////////////////////////////////
//!
//! \file Display4x7.h
//!
//! \brief 
//! Function and definition for data displaying on 4x7 LED Display using shift registers
//!
//! \author Oleg Odnoklubov "sergey@mcsys.by"
//!
////////////////////////////////////////////////////////////////////////////////
#ifndef _DISPLAY4X7_H_
#define _DISPLAY4X7_H_

#include "iostm8s105c6.h"
#include "types.h"
#include "rtc.h"
#include "delay.h"
#include "helper.h"
#include "wdt.h"
#include <math.h>
#include <stdlib.h>

//  Connection scheme to shift register:
//  PB0 -----> CLK      //  Clock impulses
//  PB1 -----> DTA      //  Data pin
//  PB2 -----> LTC      //  Latch pin

////////////////////////////////////////////////////////////////////////////////
// Macro definitions
////////////////////////////////////////////////////////////////////////////////

#define CLK_HIGH 	        PB_ODR|=MASK_PB_ODR_ODR0
#define DTA_HIGH 	        PB_ODR|=MASK_PB_ODR_ODR1
#define LTC_HIGH 	        PB_ODR|=MASK_PB_ODR_ODR2

#define CLK_LOW 	        PB_ODR&=~MASK_PB_ODR_ODR0
#define DTA_LOW		        PB_ODR&=~MASK_PB_ODR_ODR1
#define LTC_LOW		        PB_ODR&=~MASK_PB_ODR_ODR2

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

//  Init GPIO pins for Display connection
void DisplayGpioInit();

//  Shift register - 74hc595
//  Set pin values of cRegAmt successively connected 8-bit shift registes 74hc595
//  cRegAmt - amount of registers (1 - 4)
void SetShiftRegister(UINT32 sWord, UINT8 cRegAmt);

//  Display 4 unsigned digits on 4x7 display
void Display_4UnsigDigit(UINT16 sNumToDisplay, UINT32 iDisplayTimeMs);

//  Display 3 signed digits on 4x7 display - first 7-segment for sign (' '/'-')
void Display_3SigDigit(INT16 sNumToDisplay, UINT32 iDisplayTimeMs, UINT16 isNumFloat);

//  Display 'n F' - not found
void DisplayErrNotFound(UINT32 iDisplayTimeMs);

//  Display signed sNumToDisplay1 and sNumToDisplay2 on successively connected 2 4x7 Displays
void Display_8Leds(INT16 sNumToDisplay1, INT16 sNumToDisplay2, UINT16 isNumFloat);

#endif //_DISPLAY4X7_H_
