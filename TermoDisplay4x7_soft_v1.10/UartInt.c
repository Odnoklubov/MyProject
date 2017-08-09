#include "UartInt.h"

bool isUartRec = FALSE;
UINT32 ulTimeEnd = 0;

// Буфер на прием
volatile UINT8    rx_buffer[RX_BUFFER_SIZE];
volatile UINT16   rx_wr_index = 0, //индекс хвоста буфера (куда писать данные)
                    rx_rd_index = 0, //индекс начала буфера (откуда читать данные)
                    rx_counter = 0; //количество данных в буфере
volatile UINT8    rx_buffer_overflow = 0; //информация о переполнении буфера

// Буфер на передачу
volatile UINT8   tx_buffer[TX_BUFFER_SIZE];
volatile UINT16  tx_wr_index = 0, //индекс хвоста буфера (куда писать данные)
                   tx_rd_index = 0, //индекс начала буфера (откуда читать данные)
                   tx_counter = 0; //количество данных в буфере


volatile UINT8   usart_enabled = 0; // флаг разрешения приёма

void vUART2_RxProc();

void vUartInit(UINT32 ulBaudRate, UINT8 ucMode)
{ 
  // Setting GPIO for correct work as UART  
    PD_DDR |= MASK_PD_DDR_DDR5;  //pin PD5 - Tx
    PD_DDR &= ~MASK_PD_DDR_DDR6; //pin PD6 - Rx
    PD_CR1 |= MASK_PD_CR1_C15;
    PD_CR2 &= ~MASK_PD_CR1_C15;
    PD_CR1 |= MASK_PD_CR1_C16;
    PD_CR2 &= ~MASK_PD_CR1_C16;

    usart_enabled = 0;
    rx_buffer_overflow = 0;
  
    rx_wr_index = 0;
    rx_rd_index = 0;
    rx_counter = 0;
  
    tx_wr_index = 0;
    tx_rd_index = 0;
    tx_counter = 0;
  
    // set UART mode
    UART2_CR1 &= ~(MASK_UART2_CR1_M | MASK_UART2_CR1_PCEN);
    UART2_CR2 |= ucMode;

    // according ref. man. p.333
    // ulUART_DIV = 0xABCD
    // UART2_BRR2 = 0xAD
    // UART2_BRR1 = 0xBC
    UINT16 ulUART_DIV = FOSC / ulBaudRate;
    UART2_BRR2 = (UINT8)(((ulUART_DIV >> 12) & 0xF0) | (ulUART_DIV & 0x0F));
    UART2_BRR1 = (UINT8)(ulUART_DIV >> 4);

    // enable
    UART2_CR2_RIEN = 1; // enable RXNE and OR interrupts
  
    UART2_CR1 &= ~MASK_UART2_CR1_UART0; // disable low power mode
}

UINT16 usUartGetRxCount()
{
    return rx_counter;
}

UINT8 bUartGetOverflowFlag()
{
    return rx_buffer_overflow;
}

UINT16 usUartGet(UINT8 *pBuf, UINT16 nSize)
{
    // проверяем наличие чего-либо в буфере, а также корректность параметров
    if ( (rx_counter == 0) || (pBuf == 0) || !nSize )
        return 0;

    // запрещаем прерывания приёмника
    UART2_CR2_RIEN = 0;

    UINT16 nRet = rx_counter;
    if (nRet > nSize)
    {
        nRet = nSize;
    }

    // копируем данные
    for (UINT16 i = 0; i < nRet; i++)
    {
        *(pBuf+i) = rx_buffer[rx_rd_index++];

        if (rx_rd_index == RX_BUFFER_SIZE)
        {
            rx_rd_index = 0;
        }

        // Важно! Т.е. прерывания запрещены, мы должны отслеживать флаги вручную.
        // Копирование может занять достаточное время, а во время этого мы можем потерять байты
        // Чтобы этого не произошло - проверяем флаги
        // По-хорошему это нужно делать через DMA
        if ( UART2_SR_RXNE )
        {
            vUART2_RxProc();
        }
    }
//  tx_total += nRet;
    rx_counter -= nRet;

    // разрешаем прерывания приёмника
    UART2_CR2_RIEN = 1;
  
    if ( UART2_SR_RXNE )
    {
        vUART2_RxProc();
    }
  
    return nRet;
}

void vUartSetEnabled(UINT8 ucEnable)
{
    usart_enabled = ucEnable;
}

void vUartPutC(UINT8 cByte)
{
    while (tx_counter == TX_BUFFER_SIZE); // если буфер полон - подвисаем тут

    // запрещаем прерывания на время наших манипуляций  
    UART2_CR2_TIEN = 0;
  
    if (tx_counter || (!UART2_SR_TXE)) // помещать в буфер нужно в 2 случаях: либо в буфере уже что-то есть, либо передатчик занят (уже передаёт)
    {
        tx_buffer[tx_wr_index++] = cByte;

        if (tx_wr_index == TX_BUFFER_SIZE)
        {
            tx_wr_index = 0;
        }
        tx_counter++;
    
        UART2_CR2_TIEN = 1;
    }
    else
    {
        UART2_DR = cByte;
    }
}

void vUartPutS(char *str)
{
    while (*str != 0)
        vUartPutC((UINT8)*str++);
}

void vUartPutL(int x)
{
    UINT8 temp[10], count = 0;
    if (x < 0)
    {
        x = -x;
        vUartPutC('-');
    }

    if (x)
    {     
        while (x)
        {
            temp[count++] = x%10+'0';
            x /= 10;
        }

        while (count)
        {
            vUartPutC(temp[--count]);
        }
    }
    else
    {
        vUartPutC('0');
    }
}

void vUartPutBuf(void *pBuf, UINT16 nSize)
{
    for (UINT16 i = 0; i < nSize; i++)
        vUartPutC(*((UINT8*)pBuf+i));
}

void vUartPutH(UINT32 x, UINT8 n)
{
    UINT8 temp[8], count = 0;

    if (n > 8)
    {
        n = 8;
    }

    for (UINT8 i = 0; i < n; i++)
    {
        if ((x & 0xF) < 0xA)
        {
            temp[count++] = '0' + (x & 0xF);
        }
        else
        {
            temp[count++] = 'A' + (x & 0xF) - 0xA;
        }

        x >>= 4;
    }

    while (count)
    {
        vUartPutC(temp[--count]);
    }
}

void bUartFlushRxBuf()
{
    rx_wr_index = 0;
    rx_rd_index = 0;
    rx_counter = 0;
    rx_buffer_overflow = 0;
}
void bUartFlushTxBuf()
{
    tx_wr_index = 0;
    tx_rd_index = 0;
    tx_counter = 0;
}


void vUART2_RxProc()
{
    UINT8 status = UART2_SR;
    UINT8 byte = UART2_DR;
  
    if (status & MASK_UART2_SR_RXNE)
    {
        if (usart_enabled)
        {     
            rx_buffer[rx_wr_index++] = byte; //считываем данные в буфер, инкрементируя хвост буфера

            if (rx_wr_index == RX_BUFFER_SIZE)
            {
                rx_wr_index = 0; //идем по кругу
            }
            if (++rx_counter >= RX_BUFFER_SIZE) //переполнение буфера
            {
                rx_counter = RX_BUFFER_SIZE;
                usart_enabled = 0; // останавливаем приём
                rx_buffer_overflow = 1;  //сообщаем о переполнении
            }
        }
   }

//  When TEM-105 sending data, buffer is alweys owerflowing. Comment below disables owerflow interrupt.
//  if (status & MASK_UART2_SR_OR_LHE) //прерывание по переполнению буфера
//  {
//    vUartPutS("\rovf\r");
//  }
}

UINT16 ReadUartDataPacket(UINT8 *buffer, UINT16 stopByteDelay)
{
    UINT16 sUartBufLength;
    while(ulTimeEnd + ms(stopByteDelay) > GetTickCount()); //  If time since last Byte > stopByteDelay, package ends.
    sUartBufLength = usUartGetRxCount();                    //  Get length of received data
    usUartGet(buffer, sUartBufLength);                      //  Read UART RX buffer to buf array
    isUartRec = FALSE;                                      //  Set UART receive data flag to FALSE
    bUartFlushRxBuf();
    return sUartBufLength;
}

UINT8 GetIsUartRec()
{
    return isUartRec;
}

// UART TXE (Tx empty) interrupt handler function
#pragma vector = UART2_T_TXE_vector
__interrupt void vUART2_TxInt(void)
{
    if (UART2_SR & MASK_UART2_SR_TXE) //прерывание по передаче
    {
        if (tx_counter) //если есть что передать
        {
            --tx_counter; // уменьшаем количество не переданных данных
            UART2_DR = tx_buffer[tx_rd_index++]; //передаем данные инкрементируя хвост буфера
        
            if (tx_rd_index == TX_BUFFER_SIZE)
            {
                tx_rd_index = 0; //идем по кругу
            }
        }
        else 
        {// если нечего передать, запрещаем прерывание по передачи
            UART2_CR2_TIEN = 0;
        }
    }
    ulTimeEnd = GetTickCount();
} 

// UART RXNE (Rx not empty) interrupt handler function
#pragma vector = UART2_R_RXNE_vector

__interrupt void vUART2_RxInt(void)
{  
    vUART2_RxProc();  
    if(usart_enabled)
        isUartRec = TRUE;
    ulTimeEnd = GetTickCount();

}