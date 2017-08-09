#include "WirelessNetwork.h"
#include "stdlib.h"
#include "spi.h"
#include "rfm22.h"
#include "delay.h"
#include "LEDs.h"
#include "UartInt.h"
#include "rtc.h"
#include "helper.h"
#define RF_FREQ_433     //  Comment if Freq = 868
#define MODE_SPY

uint8 aTemper[2];
uint8 aRxPacket[64];
uint8 aUartPacket[150];
uint8 ucRssi = 0;
uint32 sTimePoint;
uint16 sPackNum = 0;
uint32 iNodeID = 0;
uint8 cRxPackLength = 0;
void NetworkInit(BOOL mode,uint8 MY_ID) 
{
  
    //Prepare headers.
    SPITransmitByte(TRANSMIT_HEADER_3_RW,0xff); 
    SPITransmitByte(TRANSMIT_HEADER_2_RW,0xff); 
    SPITransmitByte(TRANSMIT_HEADER_1_RW,0xff); 
    SPITransmitByte(TRANSMIT_HEADER_0_RW,0xff); 

    SPITransmitByte(HEADER_ENABLE_3_RW,BROAD);                                  //check all 32 bits of header  when recieve packet.
    SPITransmitByte(HEADER_ENABLE_2_RW,0);
    SPITransmitByte(HEADER_ENABLE_1_RW,0);
    SPITransmitByte(HEADER_ENABLE_0_RW,0);
    
    SPITransmitByte(CHECK_HEADER_3_RW,BROAD);                                   //setup header to be check
    SPITransmitByte(CHECK_HEADER_2_RW,BROAD);
    SPITransmitByte(CHECK_HEADER_1_RW,BROAD);
    SPITransmitByte(CHECK_HEADER_0_RW,BROAD);
  
#ifdef MODE_SPY
    SPITransmitByte(HEADER_CONTROL_1_RW, CHECK_HEADER_NO | CHECK_BROAD_ALL);   //OK if receive my header or broadcast header
#else
    SPITransmitByte(HEADER_CONTROL_1_RW, CHECK_HEADER_ALL | CHECK_BROAD_ALL);   //OK if receive my header or broadcast header
#endif    
    SPITransmitByte(HEADER_CONTROL_2_RW,SYNC3210 | HEAD3);                   //setup header and syncword length
    
    RFM_interrupt_RX ();

      while(1)
      {
        if(!Event_RFM)
        {
          uint8 st1 = SPIReceiveByte(INTERRUPT_STATUS_1_R);                     //reading and clering interrupt registers
          uint8 st2 = SPIReceiveByte(INTERRUPT_STATUS_2_R);                     //reading and clering interrupt registers
          
          if (st2 & IPREAVAL)
          {
            ucRssi = SPIReceiveByte(RECEIVED_SIGNAL_STRENGTH_INDICATOR_R);
            //cRxPackLength = ReceivePacket(aRxPacket);
            //asm("nop");
          }
          
          if (st1 & IPKVALID)
          {
            LED3_ON;
            cRxPackLength = ReceivePacket(aRxPacket);
                       
            if(IsArrCmp(aRxPacket, "GetTemp", 7))
            {
                snprintf((char*)aUartPacket, 7,"Term:"); 
                convertHexToStr(ucRssi, aUartPacket + 5, 1);
                convertHexToStr(aRxPacket[7], aUartPacket + 7, 1);
                convertHexToStr(aRxPacket[8], aUartPacket + 9, 1);
            }
            else
            {
                snprintf((char*)aUartPacket, 7,"Node:"); 
                convertHexToStr(ucRssi, aUartPacket + 5, 1);
                convertHexToStr(aRxPacket[0], aUartPacket + 7, 1);
                convertHexToStr(aRxPacket[1], aUartPacket + 9, 1);
            }
            
#ifdef MODE_SPY
            aUartPacket[11] = '\r';
            aUartPacket[12] = '\n';
            vUartPutBuf(aUartPacket, 13);
#else            
            iNodeID = ConvertByteArrayToInt(aRxPacket + 4);
            convertHexToStr(sPackNum++, aUartPacket, 2);                        //ConvertShortToByteArray(sPackNum++, aUartPacket);
            convertHexToStr(iNodeID, aUartPacket + 4, 4);                       //ConvertIntToByteArray(iNodeID, aUartPacket + 2);
            convertHexToStr(ucRssi, aUartPacket + 12, 1);                  //aUartPacket[6] = ucRssi;
            vUartPutBuf(aUartPacket, 14);
#endif 
            Delay_ms(100);
            LED3_OFF;
          } 
        }
      }
}