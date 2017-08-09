#include "spi.h"

#include "types.h"

void SPIInit(void)
{
    PC_DDR |= 0x20 | 0x40; // configurate SCK and MOSI(SDI) pins
    PC_CR1 |= 0x20 | 0x40; // push-pull output 
    PC_CR2 |= 0x20 | 0x40; // output speed limited to 10 MHz
  
    PC_DDR &= ~0x80; // configurate MISO(SDO) pin
  
    PC_DDR |= MASK_NSEL; // condigurate NSS(nSEL) pin;
    PC_CR1 |= MASK_NSEL; // push-pull output
    PC_CR2 |= MASK_NSEL; // output speed limited to 10 MHz
    NSEL   |= MASK_NSEL; // set up output value

    SPI_CR1 &= ~(MASK_SPI_CR1_BR | MASK_SPI_CR1_CPOL | MASK_SPI_CR1_CPHA); // Baud rate = f master/2; SCK pin has a
                                                                         // low level idle state; the first edge(rising edge)
                                                                         // on the SCK pin is the first data capture edge
  
    SPI_CR2 &= ~MASK_SPI_CR1_LSBFIRST; // MSBit is transmitted first
  
    SPI_CR2 |= MASK_SPI_CR2_SSM | MASK_SPI_CR2_SSI; // The slave device is select software
  
    SPI_CR1 |= MASK_SPI_CR1_MSTR; // Master configuration
    
    SPI_CR1 |= MASK_SPI_CR1_SPE; // enable the SPI
}

UINT8 xSPI(UINT8 byte)
{
	while(!(SPI_SR & MASK_SPI_SR_TXE)); // while Tx buffer is not empty
    SPI_DR = byte;
	while (!(SPI_SR & MASK_SPI_SR_RXNE)); // while Rx buffer is empty
	return SPI_DR;
}

void SPITransmitByte(UINT8 cAddress, UINT8 cData)
{
	cAddress |= SPI_BIT_WRITE_DIRECTION; // set up write direction for SPI

	NSEL &= ~MASK_NSEL; // select the slave device
	asm("nop");

	xSPI(cAddress);
	xSPI(cData);
 
	NSEL |= MASK_NSEL;
}

char SPIReceiveByte(UINT8 cAddress)
{
    char cData; // Data that we want to receive
  
    NSEL &= ~MASK_NSEL; // select the slave device
    asm("nop");
  
    xSPI(cAddress);
    cData = xSPI(0xFF);

    NSEL |= MASK_NSEL;
    asm("nop");

    return cData;
}

void SPIBurstReceive(UINT8 cAddress, UINT8 *pcData, UINT8 cSize)
{
    NSEL &= ~MASK_NSEL; // select the slave device
    asm("nop");
    
    xSPI(cAddress);
  
    for (UINT8 cCounter = 0; cCounter < cSize; cCounter++)
      pcData[cCounter] = xSPI(pcData[cCounter]); // transmit & receive data at the same time

    NSEL |= MASK_NSEL;
    asm("nop");
}

void SPIBurstTransmit(UINT8 cAddress, UINT8 *pcData, UINT8 cSize)
{
    cAddress |= SPI_BIT_WRITE_DIRECTION; // set up write direction for SPI
    SPIBurstReceive(cAddress, pcData, cSize);
}