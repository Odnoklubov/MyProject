///////////////////////////////////////////////////////////////////////////////
//!
//! \file UartInt.h
//!
//! \brief 
//! Module for UART interface, base level
//!
//! \author 
//!
////////////////////////////////////////////////////////////////////////////////
#ifndef __UART2_H_
#define __UART2_H_

//#include <stdint.h>
//#include <stdio.h>
#include "types.h"
#include "rtc.h"
#include "clk.h"
#include "helper.h"
#include "iostm8s105c6.h"
////////////////////////////////////////////////////////////////////////////////
// Macro definitions
////////////////////////////////////////////////////////////////////////////////

// Working modes for UART
#define MODE_TX		0x08
#define MODE_RX		0x04
#define MODE_TX_RX 	(MODE_TX | MODE_RX)

#define TX_BUFFER_SIZE 			400 // размер буфера для передачи. Большой буфер только для отладки для передачи текста
#define RX_BUFFER_SIZE 			400 // размер буфера для приёма

#define CBR_9600      9600
#define CBR_19200     19200
#define CBR_38400     38400
#define CBR_57600     57600
#define CBR_115200    115200

#define FOSC            F_CPU_Hz

//#define RTS_1                           GPIO_SetBits(GPIOA, GPIO_Pin_1)
//#define RTS_0                           GPIO_ResetBits(GPIOA, GPIO_Pin_1)
//#define TX_SIGNAL_STOP                  RTS_1
//#define TX_SIGNAL_START                 RTS_0
//#define TX_STOPPED                      RTS

// other definitions
//#define MANTISSA_SIGNS_MULTIPLY		100 // the number that mantissa should be  
// multiplied by (after comma) to get 
// the integer values

#define UART_ARRAY_TIMEOUT 100

#define UART_ENABLED    0x01
#define UART_DISABLED   0x00
////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Initialize UART2
//!
//! \param ulBaudRate - desired baud rate in bod/s (ex, 9600, 115200)
//! \param ucMode - working mode. It is may be Transmit, Receive, Transmit interrupt enable, Receive interrupt enable and other
//!  Mode may be TX - enable transmitter
//!              RX - enable receiver 
//!              TR - enable transmitter and receiver
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void vUartInit(UINT32 ulBaudRate, UINT8 ucMode);

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Return number of bytes in receive buffer of UART2
//!
//! \param none
//!
//! \return UINT16
//!
////////////////////////////////////////////////////////////////////////////////
UINT16 usUartGetRxCount();

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Allow to check, whether receive buffer overflow of UART2 occurred
//!
//! \param none
//!
//! \return 0 - no overflow. 1 - overflow occurred.
//!
////////////////////////////////////////////////////////////////////////////////
UINT8 bUartGetOverflowFlag();

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Enable or disable UART2 receive
//!
//! \param ucEnable: 0 - receiver disabled, 1 - receiver enabled
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void vUartSetEnabled(UINT8 ucEnable);

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Get received bytes from UART2
//!
//! \param pBuf - pointer to destination buffer
//!        nSize - maximum number of bytes to return
//!
//! \return Number of bytes were written to pBuf
//!
////////////////////////////////////////////////////////////////////////////////
UINT16 usUartGet(UINT8 *pBuf, UINT16 nSize);

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Transmit byte through UART2
//!
//! \param cByte - data byte
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void vUartPutC(UINT8 cByte);

void vUartPutS(char *str);
void vUartPutBuf(void *pBuf, UINT16 nSize);
void vUartPutL(int x);
void vUartPutH(UINT32 x, UINT8 n);
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Receive packet of bytes through UART2 
//!
//! \param pBuffer - pointer to data array of bytes
//! \param sStopByteDelay - last received byte time-out
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
UINT16 ReadUartDataPacket(UINT8 *pBuffer, UINT16 sStopByteDelay);

void bUartFlushRxBuf();
void bUartFlushTxBuf();
UINT8 GetIsUartRec();

#endif  // _UART_2_ 
