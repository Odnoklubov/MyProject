#include "iostm8s105c6.h"
#include "types.h"
#include "spi.h"
#include "delay.h"
#include "string.h"
#include "rfm22.h"
#include "UartInt.h"
#include "clk.h"
#include "LEDs.h"
#include "WirelessNetwork.h"

#ifndef _TYPES_H_
#include "types.h"
#include "helper.h"
#endif



void main(void)
{
  ClkInit();
  vRTCInit();
  asm("rim");
  SPIInit();
  Rfm22Init();
  vLedsInit();
  LedSetBlinky(LED1, 15, 30);                       //  Blink LED1
  vUartInit(CBR_115200, MODE_TX_RX);
  
  SPITransmitByte(AGC_OVERRIDE_1_RW,0x30);        //For optimize RF data transceive
  
  //-------------Power:
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  
  //-------------Preamble length:
  // 8bit - 0x08, 32bit - 0x20, 40bit - 0x28, 64bit - 0x40, 128bit - 0x80, 255bit - 0xFF
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x40);             //  40bit - recom1
  
  
  //-------------Preamble treshold:
  // 5bit - 0x28, 8bit - 0x40, 20bit - 0xA0, 31bit - 0xF8
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0xA0);  //  PrDetTreshold = 20bit - recom1
  
  //-------------Deviation (with Manchester) Settings in RfmInit - for 15kHz:
  //-------------10kHz 868:
  //SPITransmitByte(FREQUENCY_DEVIATION_RW,0x08);
  //SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x36);
  //--------------------
  
  //-------------5kHz 868:
  //SPITransmitByte(FREQUENCY_DEVIATION_RW,0x08);
  //SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x6B);
  //--------------------
  
  //-------------5kHz 433:
  //SPITransmitByte(FREQUENCY_DEVIATION_RW,0x08);
  //SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0xD4);
  //--------------------
  //-------------Deviation (withOUT Manchester) Settings in RfmInit - for 15kHz:
    //-------------5kHz 868:
  //SPITransmitByte(FREQUENCY_DEVIATION_RW,0x08);
  //SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x1C);
  //--------------------
  
  //-------------5kHz 433:
  //SPITransmitByte(FREQUENCY_DEVIATION_RW,0x08);
  //SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x36);
  //NetworkInit(TERMINAL,0xFE);
  NetworkInit(NODE,0x01);
}
