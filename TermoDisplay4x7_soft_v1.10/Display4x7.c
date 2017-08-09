#include "Display4x7.h"

UINT32 ToDisplayValue[8];
UINT8 cDisplayedSymbols = 0;

UINT32 DigitCodes[] = 
{
    0x03,       //  0
    0x9f,       //  1
    0x25,       //  2
    0x0d,       //  3
    0x99,       //  4
    0x49,       //  5
    0x41,       //  6
    0x1f,       //  7
    0x01,       //  8
    0x09        //  9
};

void DisplayGpioInit()
{
    //PB0 - CLK:
    PB_ODR&=~MASK_PB_ODR_ODR0;   // output to 0 
    PB_CR1|=MASK_PB_CR1_C10;     // push-pull output
    PB_CR2&=~MASK_PB_CR2_C20;    // low speed output
    PB_DDR|=MASK_PB_DDR_DDR0;    // as output
    //PB1 - DTA:
    PB_ODR&=~MASK_PB_ODR_ODR1;   // output to 0 
    PB_CR1|=MASK_PB_CR1_C11;     // push-pull output
    PB_CR2&=~MASK_PB_CR2_C21;    // low speed output
    PB_DDR|=MASK_PB_DDR_DDR1;    // as output
    //PB2 - LTC:
    PB_ODR&=~MASK_PB_ODR_ODR2;   // output to 0 
    PB_CR1|=MASK_PB_CR1_C12;     // push-pull output
    PB_CR2&=~MASK_PB_CR2_C22;    // low speed output
    PB_DDR|=MASK_PB_DDR_DDR2;    // as output
    
    //SetShiftRegister(0x00000000, 4);                                            //  Clear screen
    ToDisplayValue[0] = 0x10490000;
    ToDisplayValue[1] = 0x20e10000;
    ToDisplayValue[2] = 0x40110000;
    ToDisplayValue[3] = 0x80f50000;
    ToDisplayValue[4] = 0x10e1;
    cDisplayedSymbols = 5;
}

void SetShiftRegister(UINT32 sWord, UINT8 cRegAmt)
{
    //  Start settings:
    LTC_LOW;
    CLK_LOW;
    DTA_LOW;
    
    //  Profilacting latch
    Delay_us(10);
    LTC_HIGH;
    Delay_us(20);
    
    //  Start filling registers pins
    LTC_LOW;
    for(UINT8 i = 0; i < (8 * cRegAmt); i++)                                    //sent 8bit data
    {//  Until last register was reached - like SPI
        Delay_us(10);
        CLK_LOW;    
        if(sWord & 0x01)
            DTA_HIGH;//LSB first
        else 
            DTA_LOW;
        sWord >>= 1;  
        CLK_HIGH;
    }  
    LTC_HIGH;                                                                   //  Latch event - apply sWord to register pins
    Delay_us(10);
}

void Display_4UnsigDigit(UINT16 sNumToDisplay, UINT32 iDisplayTimeMs)
{
    UINT8 cDIgitCount = GetDigitNum(sNumToDisplay);
    UINT32 iTickCount = GetTickCount();
    while(!IsTickPassed(iTickCount + ms(iDisplayTimeMs)))
    {
        switch(cDIgitCount)
        {
            case 0:
            case 1:
              SetShiftRegister(0x8000 | DigitCodes[sNumToDisplay], 2);
              break;
            case 2:
              SetShiftRegister(0x8000 | DigitCodes[sNumToDisplay%10], 2);
              SetShiftRegister(0x4000 | DigitCodes[(sNumToDisplay/10)%10], 2);
              break;
            case 3:
              SetShiftRegister(0x8000 | DigitCodes[sNumToDisplay%10], 2);
              SetShiftRegister(0x4000 | DigitCodes[(sNumToDisplay/10)%10], 2);
              SetShiftRegister(0x2000 | DigitCodes[(sNumToDisplay/100)%10], 2);
              break;
            case 4:
              SetShiftRegister(0x8000 | DigitCodes[sNumToDisplay%10], 2);
              SetShiftRegister(0x4000 | DigitCodes[(sNumToDisplay/10)%10], 2);
              SetShiftRegister(0x2000 | DigitCodes[(sNumToDisplay/100)%10], 2);
              SetShiftRegister(0x1000 | DigitCodes[(sNumToDisplay/1000)%10], 2);
              break;
            default:
              //  Show error message
              SetShiftRegister(0x8000 | 0xf5, 2);                               //  r
              SetShiftRegister(0x4000 | 0xf5, 2);                               //  r
              SetShiftRegister(0x2000 | 0x61, 2);                               //  E
              break;
        }
    }
  
}

void Display_3SigDigit(INT16 sNumToDisplay, UINT32 iDisplayTimeMs, UINT16 isNumFloat)
{
    UINT8 cDIgitCount = GetDigitNum(sNumToDisplay);
    //UINT8 cDisplayedSymbols = 0;
    UINT32 iTickCount = 0;

    switch(cDIgitCount)
    {//  Define ToDisplayValue words for all Displayed digits
        case 0:
        case 1:
          if(sNumToDisplay < 0)
          {
              if(!isNumFloat)
              {
                  ToDisplayValue[0] = 0x8000 | DigitCodes[abs(sNumToDisplay)];
                  ToDisplayValue[1] = 0x4000 | 0xfd;                            //  Minus
                  cDisplayedSymbols = 2;    
              }
              else
              {
                  ToDisplayValue[0] = 0x8000 | DigitCodes[abs(sNumToDisplay)];
                  ToDisplayValue[1] = 0x4000 | DigitCodes[0] & ~isNumFloat;
                  ToDisplayValue[2] = 0x2000 | 0xfd;                            //  Minus
                  cDisplayedSymbols = 3;
              }
          }
          else
          {
              if(!isNumFloat)
              {
                  ToDisplayValue[0] = 0x8000 | DigitCodes[sNumToDisplay];
                  cDisplayedSymbols = 1;
              }
              else
              {
                  ToDisplayValue[0] = 0x8000 | DigitCodes[abs(sNumToDisplay)];
                  ToDisplayValue[1] = 0x4000 | DigitCodes[0] & ~isNumFloat;
                  cDisplayedSymbols = 2;
              }
          }
          break;
        case 2:
          if(sNumToDisplay < 0)
          {
              ToDisplayValue[0] = 0x8000 | DigitCodes[abs(sNumToDisplay)%10];
              ToDisplayValue[1] = 0x4000 | DigitCodes[(abs(sNumToDisplay)/10)%10] & ~isNumFloat;
              ToDisplayValue[2] = 0x2000 | 0xfd;                                //  Minus
              cDisplayedSymbols = 3;
          }
          else
          {
              ToDisplayValue[0] = 0x8000 | DigitCodes[sNumToDisplay%10];
              ToDisplayValue[1] = 0x4000 | DigitCodes[(sNumToDisplay/10)%10] & ~isNumFloat;
              cDisplayedSymbols = 2;
          }
          break;
        case 3:
          if(sNumToDisplay < 0)
          {
              ToDisplayValue[0] = 0x8000 | DigitCodes[abs(sNumToDisplay)%10];
              ToDisplayValue[1] = 0x4000 | DigitCodes[(abs(sNumToDisplay)/10)%10] & ~isNumFloat;
              ToDisplayValue[2] = 0x2000 | DigitCodes[(abs(sNumToDisplay)/100)%10];
              ToDisplayValue[3] = 0x1000 | 0xfd;                                //  Minus
              cDisplayedSymbols = 4;
          }
          else
          {
              ToDisplayValue[0] = 0x8000 | DigitCodes[sNumToDisplay%10];
              ToDisplayValue[1] = 0x4000 | DigitCodes[(sNumToDisplay/10)%10] & ~isNumFloat;
              ToDisplayValue[2] = 0x2000 | DigitCodes[(sNumToDisplay/100)%10];
              cDisplayedSymbols = 3;
          }
          break;
        default:
          ToDisplayValue[0] = 0x8000 | 0xf5; 
          ToDisplayValue[1] = 0x4000 | 0xf5;
          ToDisplayValue[2] = 0x2000 | 0x61;
          cDisplayedSymbols = 3;
          break;
    }
    
    //  Displaying:
    iTickCount = GetTickCount();
    while(!IsTickPassed(iTickCount + ms(iDisplayTimeMs)))
    {
        for(UINT8 i = 0; i < cDisplayedSymbols; i++)
        {//  Alternately show all digits
            SetShiftRegister(ToDisplayValue[i], 2);
        }
    }
    SetShiftRegister(0x0000, 2);                                                //  Clear screen
}

void Display_8Leds(INT16 sNumToDisplay2, INT16 sNumToDisplay1, UINT16 isNumFloat)
{
    UINT8 cDIgitCount1 = GetDigitNum(sNumToDisplay1);
    UINT8 cDIgitCount2 = GetDigitNum(sNumToDisplay2);
    //UINT8 cDisplayedSymbols = 0;
   
    
    TIM3_IER_UIE = 0; //  Disable RTC Interrupt

    switch(cDIgitCount1)
    {//  Define ToDisplayValue words for all Displayed digits of sNumToDisplay1
        case 0:
        case 1:
          if(sNumToDisplay1 < 0)
          {
              if(!isNumFloat)
              {
                  ToDisplayValue[0] = 0x8000 | DigitCodes[abs(sNumToDisplay1)];
                  ToDisplayValue[1] = 0x4000 | 0xfd;                            //  Minus
                  cDisplayedSymbols = 2;    
              }
              else
              {
                  ToDisplayValue[0] = 0x8000 | DigitCodes[abs(sNumToDisplay1)];
                  ToDisplayValue[1] = 0x4000 | DigitCodes[0] & ~isNumFloat;
                  ToDisplayValue[2] = 0x2000 | 0xfd;                            //  Minus
                  cDisplayedSymbols = 3;
              }
          }
          else
          {
              if(!isNumFloat)
              {
                  ToDisplayValue[0] = 0x8000 | DigitCodes[sNumToDisplay1];
                  cDisplayedSymbols = 1;
              }
              else
              {
                  ToDisplayValue[0] = 0x8000 | DigitCodes[abs(sNumToDisplay1)];
                  ToDisplayValue[1] = 0x4000 | DigitCodes[0] & ~isNumFloat;
                  cDisplayedSymbols = 2;
              }
          }
          break;
        case 2:
          if(sNumToDisplay1 < 0)
          {
              ToDisplayValue[0] = 0x8000 | DigitCodes[abs(sNumToDisplay1)%10];
              ToDisplayValue[1] = 0x4000 | DigitCodes[(abs(sNumToDisplay1)/10)%10] & ~isNumFloat;
              ToDisplayValue[2] = 0x2000 | 0xfd;                                //  Minus
              cDisplayedSymbols = 3;
          }
          else
          {
              ToDisplayValue[0] = 0x8000 | DigitCodes[sNumToDisplay1%10];
              ToDisplayValue[1] = 0x4000 | DigitCodes[(sNumToDisplay1/10)%10] & ~isNumFloat;
              cDisplayedSymbols = 2;
          }
          break;
        case 3:
          if(sNumToDisplay1 < 0)
          {
              ToDisplayValue[0] = 0x8000 | DigitCodes[abs(sNumToDisplay1)%10];
              ToDisplayValue[1] = 0x4000 | DigitCodes[(abs(sNumToDisplay1)/10)%10] & ~isNumFloat;
              ToDisplayValue[2] = 0x2000 | DigitCodes[(abs(sNumToDisplay1)/100)%10];
              ToDisplayValue[3] = 0x1000 | 0xfd;                                //  Minus
              cDisplayedSymbols = 4;
          }
          else
          {
              ToDisplayValue[0] = 0x8000 | DigitCodes[sNumToDisplay1%10];
              ToDisplayValue[1] = 0x4000 | DigitCodes[(sNumToDisplay1/10)%10] & ~isNumFloat;
              ToDisplayValue[2] = 0x2000 | DigitCodes[(sNumToDisplay1/100)%10];
              cDisplayedSymbols = 3;
          }
          break;
        default:
          ToDisplayValue[0] = 0x8000 | 0xf5; 
          ToDisplayValue[1] = 0x4000 | 0xf5;
          ToDisplayValue[2] = 0x2000 | 0x61;
          cDisplayedSymbols = 3;
          break;
    }
              
    switch(cDIgitCount2)
    {//  Define ToDisplayValue words for all Displayed digits of sNumToDisplay2
        case 0:
        case 1:
          if(sNumToDisplay2 < 0)
          {
              if(!isNumFloat)
              {
                  ToDisplayValue[cDisplayedSymbols] = 0x80000000 | DigitCodes[abs(sNumToDisplay2)] << 16;
                  ToDisplayValue[cDisplayedSymbols + 1] = 0x40fd0000;           //  Minus
                  cDisplayedSymbols += 2;    
              }
              else
              {
                  ToDisplayValue[cDisplayedSymbols] = 0x80000000 | DigitCodes[abs(sNumToDisplay2)] << 16;
                  ToDisplayValue[cDisplayedSymbols + 1] = 0x40000000 | (DigitCodes[0] & ~isNumFloat) << 16;
                  ToDisplayValue[cDisplayedSymbols + 2] = 0x20fd0000;           //  Minus
                  cDisplayedSymbols += 3;
              }
          }
          else
          {
              if(!isNumFloat)
              {
                  ToDisplayValue[cDisplayedSymbols] = 0x80000000 | DigitCodes[sNumToDisplay2] << 16;
                  cDisplayedSymbols += 1;
              }
              else
              {
                  ToDisplayValue[cDisplayedSymbols] = 0x80000000 | DigitCodes[abs(sNumToDisplay2)] << 16;
                  ToDisplayValue[cDisplayedSymbols + 1] = 0x40000000 | (DigitCodes[0] & ~isNumFloat) << 16;
                  cDisplayedSymbols += 2;
              }
          }
          break;
        case 2:
          if(sNumToDisplay2 < 0)
          {
              ToDisplayValue[cDisplayedSymbols] = 0x80000000 | DigitCodes[abs(sNumToDisplay2)%10] << 16;
              ToDisplayValue[cDisplayedSymbols + 1] = 0x40000000 | (DigitCodes[(abs(sNumToDisplay2)/10)%10] & ~isNumFloat) << 16;
              ToDisplayValue[cDisplayedSymbols + 2] = 0x20fd0000;               //  Minus
              cDisplayedSymbols += 3;
          }
          else
          {
              ToDisplayValue[cDisplayedSymbols] = 0x80000000 | DigitCodes[sNumToDisplay2%10] << 16;
              ToDisplayValue[cDisplayedSymbols + 1] = 0x40000000 | (DigitCodes[(sNumToDisplay2/10)%10] & ~isNumFloat) << 16;
              cDisplayedSymbols += 2;
          }
          break;
        case 3:
          if(sNumToDisplay2 < 0)
          {
              ToDisplayValue[cDisplayedSymbols] = 0x80000000 | DigitCodes[abs(sNumToDisplay2)%10] << 16;
              ToDisplayValue[cDisplayedSymbols + 1] = 0x40000000 | (DigitCodes[(abs(sNumToDisplay2)/10)%10] & ~isNumFloat) << 16;
              ToDisplayValue[cDisplayedSymbols + 2] = 0x20000000 | DigitCodes[(abs(sNumToDisplay2)/100)%10] << 16;
              ToDisplayValue[cDisplayedSymbols + 3] = 0x10fd0000;               //  Minus
              cDisplayedSymbols += 4;
          }
          else
          {
              ToDisplayValue[cDisplayedSymbols] = 0x80000000 | DigitCodes[sNumToDisplay2%10] << 16;
              ToDisplayValue[cDisplayedSymbols + 1] = 0x40000000 | (DigitCodes[(sNumToDisplay2/10)%10] & ~isNumFloat) << 16;
              ToDisplayValue[cDisplayedSymbols + 2] = 0x20000000 | DigitCodes[(sNumToDisplay2/100)%10] << 16;
              cDisplayedSymbols += 3;
          }
          break;
        default:
          ToDisplayValue[cDisplayedSymbols] = 0x80f50000; 
          ToDisplayValue[cDisplayedSymbols + 1] = 0x40f50000;
          ToDisplayValue[cDisplayedSymbols + 2] = 0x20610000;
          cDisplayedSymbols += 3;
          break;
    }
    
    TIM3_IER_UIE = 1; //  Enable RTC Interrupt
    
    //  Displaying:
    /*iTickCount1 = GetTickCount();
    while(!IsTickPassed(iTickCount1 + ms(iDisplayTimeMs)))
    {//  Alternately show all digits
        asm("nop");
        for(UINT8 i = 0; i < cDisplayedSymbols; i++)
        {
            SetShiftRegister(ToDisplayValue[i], 4);
        }
        asm("nop");
    }
    SetShiftRegister(0x00000000, 4);                                            //  Clear screen*/
}

void DisplayErrNotFound(UINT32 iDisplayTimeMs)
{
    UINT32 iTickCount = GetTickCount();
    while(!IsTickPassed(iTickCount + ms(iDisplayTimeMs)))
    {
        SetShiftRegister(0x8000 | 0x71, 2);                                     //  F
        SetShiftRegister(0x4000 | 0xd5, 2);                                     //  n
    }
    SetShiftRegister(0x0000, 2);                                                //  Clear screen
}
