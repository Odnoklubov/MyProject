#include "spi.h"

void SPIInit(void)
{
    PC_DDR |= 0x20 | 0x40; // configurate SCK and MOSI(SDI) pins
    PC_CR1 |= 0x20 | 0x40; // push-pull output 
    PC_CR2 |= 0x20 | 0x40; // output speed limited to 10 MHz
  
    PC_DDR &= ~0x80; // configurate MISO(SDO) pin
  
    PC_DDR |= MASK_NSEL; // condigurate NSS(nSEL) pin;
    PC_CR1 |= MASK_NSEL; // push-pull output
    PC_CR2 |= MASK_NSEL; // output speed limited to 10 MHz
    NSEL |= MASK_NSEL; // set up output value

    SPI_CR1 &= ~(MASK_SPI_CR1_BR | MASK_SPI_CR1_CPOL | MASK_SPI_CR1_CPHA); // Baud rate = f master/2; SCK pin has a
                                                                         // low level idle state; the first edge(rising edge)
                                                                         // on the SCK pin is the first data capture edge
  
    SPI_CR2 &= ~MASK_SPI_CR1_LSBFIRST; // MSBit is transmitted first
  
    SPI_CR2 |= MASK_SPI_CR2_SSM | MASK_SPI_CR2_SSI; // The slave device is select software
  
    SPI_CR1 |= MASK_SPI_CR1_MSTR; // Master configuration
}

void SPITransmitByte(char cAddress, char cData)
{
    asm("sim");                 // for disable all others interrupt
    cAddress |= SPI_SET_WRITE_DIRECTION; // set up write direction for SPI
  
    NSEL &= ~MASK_NSEL; // select the slave device
    SPI_CR1 |= MASK_SPI_CR1_SPE; // enable the SPI
  
    while(!(SPI_SR & MASK_SPI_SR_TXE)); // while Tx buffer is not empty
    SPI_DR = cAddress; // write the address were we want to transmit the data
  
    while(!(SPI_SR & MASK_SPI_SR_TXE)); // while Tx buffer is not empty
    SPI_DR = cData; // write the data
  
    while(!(SPI_SR & MASK_SPI_SR_RXNE)); // while Rx buffer is empty
    cAddress = SPI_DR; // read something
 
    while(!(SPI_SR & MASK_SPI_SR_RXNE)); // while Rx buffer is empty
    cAddress = SPI_DR; // read something
  
    while(!(SPI_SR & MASK_SPI_SR_TXE) || (SPI_SR & MASK_SPI_SR_BSY)); // wait until the completion of the transfer.
  
    SPI_CR1 &= ~MASK_SPI_CR1_SPE; // disable the SPI
  
    NSEL |= MASK_NSEL;
    asm("rim");                 // for enable all others interrupt
}

char SPIReceiveByte(char cAddress)
{
    asm("sim");                 // for disable all others interrupt
    char cData; // Data that we want to receive
  
    NSEL &= ~MASK_NSEL; // select the slave device
    SPI_CR1 |= MASK_SPI_CR1_SPE; // enable the SPI
  
    while(!(SPI_SR & MASK_SPI_SR_TXE)); // while Tx buffer is not empty
    SPI_DR = cAddress; // write the address from where we want to receive data
  
    while(!(SPI_SR & MASK_SPI_SR_TXE)); // while Tx buffer is not empty
    SPI_DR = 0x00; // write something
  
    while(!(SPI_SR & MASK_SPI_SR_RXNE)); // while Rx buffer is empty
    cData = SPI_DR; // read something
  
    while(!(SPI_SR & MASK_SPI_SR_RXNE)); // while Rx buffer is empty
    cData = SPI_DR; // read the requested data
  
    while(!(SPI_SR & MASK_SPI_SR_TXE) || (SPI_SR & MASK_SPI_SR_BSY)); // wait until the completion of the transfer.
  
    SPI_CR1 &= ~MASK_SPI_CR1_SPE; // disable the SPI
    NSEL |= MASK_NSEL;
  
    asm("rim");                 // for enable all others interrupt
    return cData; 
}

void SPIBurstReceive(char cAddress, char *pcData, char cSize)
{
    char cCounter = 0;
  
    NSEL &= ~MASK_NSEL; // select the slave device
    SPI_CR1 |= MASK_SPI_CR1_SPE; // enable the SPI
  
    while(!(SPI_SR & MASK_SPI_SR_TXE)); // while Tx buffer is not empty
    SPI_DR = cAddress; // write the address
  
    while(!(SPI_SR & MASK_SPI_SR_RXNE)); // while Rx buffer is empty
    cAddress = SPI_DR; // receive data
  
    for(cCounter = 0; cCounter < cSize; cCounter++) {
        while(!(SPI_SR & MASK_SPI_SR_TXE)); // while Tx buffer is not empty
        SPI_DR = pcData[cCounter]; // transmit data
        while(!(SPI_SR & MASK_SPI_SR_RXNE)); // while Rx buffer is empty
        pcData[cCounter] = SPI_DR; // receive data
    }
  
    while(!(SPI_SR & MASK_SPI_SR_TXE) || (SPI_SR & MASK_SPI_SR_BSY)); // wait until the completion of the transfer.

    SPI_CR1 &= ~MASK_SPI_CR1_SPE; // disable the SPI
    NSEL |= MASK_NSEL;
}

void SPIBurstTransmit(char cAddress, char *pcData, char cSize)
{
    cAddress |= SPI_SET_WRITE_DIRECTION; // set up write direction for SPI
    SPIBurstReceive(cAddress,pcData,cSize);
}