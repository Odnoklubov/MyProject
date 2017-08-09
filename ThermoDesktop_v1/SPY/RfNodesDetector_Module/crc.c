#include "crc.h"

char Crc8(char *pcMessage, char cLength)
{
    char cCrc = CRC8_INIT; // CRC initialization
    char i = INIT_ZERO;
    
    while(cLength--)
    {
        // In this loop in every iteration calculating CRC for the next byte 
        
        cCrc ^= *pcMessage++; // Calculating exclusive-OR of the CRC value with the next byte of message
        
        // CRC shift left to one bit. If 7 bit equal "1" after shift
        // calculates xor CRC with the polynomial
        for(i = CLEAR_TO_ZERO; i < BITS_IN_BYTE; i++)
            if(cCrc & MASK_7_BIT) cCrc = (cCrc << SHIFT_TO_ONE_BIT) ^ CRC8_POLYNOMIAL;
            else cCrc = cCrc << SHIFT_TO_ONE_BIT;
    }
    
    return cCrc;
}

unsigned short Crc16(char *pcMessage, unsigned short usLength)
{
    unsigned short usCrc = CRC16_INIT; // CRC initialization
    char i = INIT_ZERO;
    
    while(usLength--)
    {
        // In this loop in every iteration calculating CRC for the next byte
        
        usCrc ^= *pcMessage++ << 8; // Calculates exclusive-OR of the CRC value with the next byte of message
        
        // CRC shift left to one bit. If 15 bit equal "1" after shift
        // calculates xor CRC with the polynomial
        for(i = CLEAR_TO_ZERO; i < BITS_IN_BYTE; i++)
            if(usCrc & MASK_15_BIT)
                usCrc = (usCrc << SHIFT_TO_ONE_BIT)^CRC16_POLYNOMIAL;
            else usCrc = (usCrc << SHIFT_TO_ONE_BIT);
    }
    
    return usCrc;
}