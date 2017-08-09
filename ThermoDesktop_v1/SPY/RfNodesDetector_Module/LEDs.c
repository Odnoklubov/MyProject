#include "LEDs.h"

#include "types.h"

LED_MODE ucMode[4]    = {LED_MODE_CONSTANT, LED_MODE_CONSTANT, LED_MODE_CONSTANT, LED_MODE_CONSTANT};
//BOOL abState[4]       = {FALSE, FALSE, FALSE, FALSE};
UINT8 ucTonPulses[4]  = {0, 0, 0, 0};
UINT8 ucToff[4]       = {10, 10, 10, 10};
UINT8 ucStep[4]       = {0, 0, 0, 0};

const UINT16 ucLedReg[4] = {0x5014, 0x5014, 0x5014, 0x500A}; // ODR registers addresses
const UINT8 ucLedRegMask[4] = {MASK_PE_ODR_ODR7, MASK_PE_ODR_ODR6, MASK_PE_ODR_ODR5, MASK_PC_ODR_ODR1};

void vLedsInit()
{
    // LEDs
    // PE7 - LED D1
    PE_DDR_DDR7 = 1;
    PE_CR1_C17  = 1;
    PE_CR2_C27  = 0;
    //LED1_OFF;

    // PE6 - LED D2
    PE_DDR_DDR6 = 1;
    PE_CR1_C16  = 1;
    PE_CR2_C26  = 0;
    //LED2_OFF;
  
    // PE5 - LED D3
    PE_DDR_DDR5 = 1;
    PE_CR1_C15  = 1;
    PE_CR2_C25  = 0;
    //LED3_OFF;
  
    // PC1 - LED D4
    PC_DDR_DDR1 = 1;
    PC_CR1_C11  = 1;
    PC_CR2_C21  = 0;
    //LED4_OFF;
  
    // Set timer for our LEDs
    TIM3_ARRH = (UINT8)(1562 >> 8); // TIM3 auto-reload register. Border for the interrupt appearance.
    TIM3_ARRL = (UINT8)(1562 >> 0);

    TIM3_PSCR_PSC = TIM_PRESCALER_1024; // Fosc/1024

    TIM3_IER     = 0;
    TIM3_IER_UIE = 1; // enable interrupt

    TIM3_CR1 = 0;
    TIM3_CR1_ARPE = 1; // auto-reload enable

    TIM3_CR1_CEN  = 1; // enable timer
}

#pragma vector = TIM3_OVR_UIF_vector
__interrupt void vLedsInterrupt()
{
    for (UINT8 i = 0; i < 4; i++)
    {
        if (LED_MODE_BLINKY == ucMode[i])
        {
            ucStep[i]++;
      
            if (ucStep[i] == ucTonPulses[i])
            {
                *((UINT8*)ucLedReg[i]) &= ~ucLedRegMask[i]; // LED off
            }
            else if (ucStep[i] >= ucToff[i])
            {
                ucStep[i] = 0;
                if (ucTonPulses[i])
                {
                    *((UINT8*)ucLedReg[i]) |= ucLedRegMask[i]; // LED on
                }
            }
        }
        else if (LED_MODE_N == ucMode[i])
        {
            ucStep[i]++;
      
            if (ucStep[i] & 0x01)
            {
                *((UINT8*)ucLedReg[i]) |= ucLedRegMask[i]; // LED on
            }
            else
            {
                *((UINT8*)ucLedReg[i]) &= ~ucLedRegMask[i]; // LED off
                if (--ucTonPulses[i] == 0)
                {
                    ucMode[i] = LED_MODE_CONSTANT;
                }
            }
        }
    }

    TIM3_SR1_UIF = 0; // Clear interrupt flag
}

void LedSet(LEDn ucLed, BOOL bState)
{
    ucMode[ucLed] = LED_MODE_CONSTANT; // set LED mode
//  abState[ucLed] = bState;

    if (bState)
    {
        *((UINT8*)ucLedReg[ucLed]) |= ucLedRegMask[ucLed]; // LED on
    }
    else
    {
        *((UINT8*)ucLedReg[ucLed]) &= ~ucLedRegMask[ucLed]; // LED off
    }
}

void LedSetBlinky(LEDn ucLed, UINT8 ucTon_, UINT8 ucToff_)
{
    UINT8 ucUpd = 0; // used to decide wheter we need to "reset" LED cycle

    if (ucTon_ == 0)
    {
        LedSet(ucLed, FALSE);
        return;
    }

    if (ucToff_ == 0)
    {
        LedSet(ucLed, TRUE);
        return;
    }

    if (ucMode[ucLed] != LED_MODE_BLINKY)
    {
        ucUpd = 1;
        ucMode[ucLed] = LED_MODE_BLINKY; // set LED mode
    }

    if (ucTonPulses[ucLed] != ucTon_)
    {
        ucUpd = 1;
        ucTonPulses[ucLed] = ucTon_; // set new Ton time
    }
  
    if (ucToff[ucLed] != ucTon_ + ucToff_)
    {
        ucUpd = 1;
        ucToff[ucLed] = ucTon_ + ucToff_;
    }
  
    if (ucUpd) // reset LED only if req. That's we can call vLedsSet frequently and LED will flash without change
    {
        ucStep[ucLed] = 0;
  
        if (ucTonPulses[ucLed])
        {
            *((UINT8*)ucLedReg[ucLed]) |= ucLedRegMask[ucLed]; // LED on
        }
        else
        {
            *((UINT8*)ucLedReg[ucLed]) &= ~ucLedRegMask[ucLed]; // LED off
        }
    }

/*  switch (ucLed)
  {
    case LED1:
     LED1_OFF;
     break;
    case LED2:
     LED2_OFF;
     break;
    case LED3:
     LED3_OFF;
     break;
    case LED4:
     LED4_OFF;
     break;
  }*/
}

void LedFlush(LEDn ucLed, UINT8 nCount)
{
    // if no flushed need - go to OFF state
    if (!nCount)
    {
        LedSet(ucLed, FALSE);
        return;
    }

    ucMode[ucLed] = LED_MODE_N; // set LED mode
    ucTonPulses[ucLed] = nCount;
    ucStep[ucLed] = 0;

    *((UINT8*)ucLedReg[ucLed]) &= ~ucLedRegMask[ucLed]; // LED off
}