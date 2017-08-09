#include "iostm8s105c6.h"
#include "types.h"
#include "spi.h"
#include "delay.h"
#include "string.h"
#include "rfm22.h"
#include "rtc.h"
#include "clk.h"
#include "LEDs.h"
#include "ds18b20.h"
#include "helper.h"
#include "Display4x7.h"
#include "wdt.h"

#include <stdlib.h>
#include <stdio.h>

#ifndef _TYPES_H_
#include "types.h"
#endif

//------- Choose device mode --------
#define MODE_TERMINAL           0x00            //  Terminal - device with screen
#define MODE_NODE               0x01            //  Node - device with sensor only
#define TEMP_REFRESH_TIME_MS    20000           //  Time of Temperature refreshing

UINT8 cMode = MODE_TERMINAL;
//-----------------------------------

UINT32 iTickCount;

UINT8 aRfBuf[64];
UINT8 cRfBufLength = 0;
INT16 sTerminalTempOld = 0;         //  Old Temperature of Terminal Device
//INT16 sNodeTempOld = 0;             //  Old Temperature of Node Device

INT16 sNodeTemperature = 0;         //  Temperature of Node Device
INT32 sNodeTempMul = 0;         //  Temperature of Node Device
INT16 sTerminalTemperature = 0;     //  Temperature of Terminal Device
bool isNodeOnLine = FALSE;          //  TRUE if node detected and online
bool isRfReceived = FALSE;
bool isRfEnabled = FALSE;
INT16 sTempMin = 0, sTempMax = 0;

INT16 asNodeTempArchive[13];

void main(void)
{
    ClkInit();
    RTCInit();
    WdtInit(WDT_TIMEOUT_1s02);
    asm("rim");

    SPIInit();
    Rfm22Init();
    LEDs_Init();
    ThermInitAll(THERM_MODE_11BIT);
    
    if(cMode==MODE_TERMINAL)                                                    
    { 
        DisplayGpioInit();
        Rfm22TerminalInit();
        while(1)
        {
            WdtReset();
            sTerminalTemperature = GetCommonTempValue();
            for(UINT8 cAttemptCount = 0; cAttemptCount < 3; cAttemptCount++)
            {
                TX_ON(); 
                WdtReset();
                snprintf((char*)aRfBuf, 10,"GetTemp");                          //  Set "GetTemp" to RF buffer
                ConvertShortToByteArray(sTerminalTemperature, aRfBuf + 7);      //  Last 2 bytes - Terminal Temperature
                
                LED3_ON;
                Delay_ms(30);
                LED3_OFF;
                
                iTickCount = GetTickCount();
                isRfReceived = FALSE;
                isRfEnabled = TRUE;
                
                SendPacket(aRfBuf,9);                                           //  Send
                RFM_interrupt_RX();
                              
                while(!IsTickPassed(iTickCount + ms(1000)))
                {//  Wait 2 seconds for Node response
                    if(isRfReceived)
                    {
                        sNodeTemperature = ConvertByteArrayToShort(aRfBuf);
                        isNodeOnLine = TRUE;
                        isRfReceived = FALSE;
                        LED4_ON;
                        Delay_ms(30);
                        LED4_OFF;
                        break;
                    }
                    WdtReset();
                }
                if(isNodeOnLine)
                  break;
            }

            if(!isNodeOnLine)
            {//  Set temperature to unreal value for 'Err' displaying
                sNodeTemperature = -10000;
            }
            
            if(abs(sTerminalTempOld - sTerminalTemperature) > 30)
            {//  If terminal temperature changes more than 3 degree - read again
                sTerminalTemperature = GetCommonTempValue();
            }
            
            Display_8Leds(sNodeTemperature, sTerminalTemperature, 1);           //  Display temperature values
            
            Delay_ms(TEMP_REFRESH_TIME_MS);
            
            isNodeOnLine = FALSE;
            sTerminalTempOld = sTerminalTemperature;
        }
    }
    else
    { 
        UINT8 i = 0;
        Rfm22NodeInit();
        sNodeTemperature = GetCommonTempValue();
        sTempMin = 0; 
        sTempMax = 0;
        sNodeTempMul = 0;
        iTickCount = GetTickCount();
        while(1)
        {
            WdtReset();
            RFM_interrupt_RX();
            isRfEnabled = TRUE;
            isRfReceived = FALSE;
            while(!IsTickPassed(iTickCount + ms(3000)))
            {
                WdtReset();
                if(isRfReceived)          
                {
                    //isRfReceived = TRUE;
                    break;
                }
            }
            
            if(!isRfReceived)
            {
                asNodeTempArchive[i] = GetCommonTempValue();
                
                if(i == 0)
                {
                    sTempMin = asNodeTempArchive[i]; 
                    sTempMax = asNodeTempArchive[i];
                }
                
                if(asNodeTempArchive[i] > sTempMax)
                {
                    sTempMax = asNodeTempArchive[i];
                }
                
                if(asNodeTempArchive[i] < sTempMin)
                {
                    sTempMin = asNodeTempArchive[i];
                }
                
                sNodeTempMul += asNodeTempArchive[i];
                
                if(++i > 11)
                {
                    sNodeTempMul = sNodeTempMul - sTempMax - sTempMin;
                    if(sNodeTempMul % 10 > 5)
                    {
                        sNodeTemperature = sNodeTempMul / 10 + 1;
                    }
                    else
                    {
                        sNodeTemperature = sNodeTempMul / 10;
                    }
                    i = 0;
                    sTempMin = 0; 
                    sTempMax = 0;
                    sNodeTempMul = 0;
                }

                iTickCount = GetTickCount();
            }
            else
            {
                //cRfBufLength=ReceivePacket(aRfBuf);
                LED2_ON;
                Delay_ms(30);
                LED2_OFF;
                //isRfReceived = FALSE;
                if(9 == cRfBufLength && strCompareLen(aRfBuf,"GetTemp",cRfBufLength - 2))
                {//  Receive - 9 bytes. two last - temperature of Terminal
                    ConvertShortToByteArray(sNodeTemperature, aRfBuf);
                    
                    TX_ON(); 
                    
                    SendPacket(aRfBuf,2);
                    LED4_ON;
                    Delay_ms(30);
                    LED4_OFF;
                } 
                else
                {
                    RFM_interrupt_RX();
                }
            }
        }
    
    }   
}
