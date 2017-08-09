#include "one-wire.h" 

void OWHigh(void) 
{
  OW_ODR |= OW_BUSMASK; // drive output high
}

 void OWLow(void) 
{
  OW_ODR &= ~OW_BUSMASK; // drive output low
}

 void OWInputMode(void)
{
  OW_DDR &= ~OW_BUSMASK; // select input mod for the pin
}

 void OWOutputMode(void)
{
  OW_DDR |= OW_BUSMASK; // select output mod for the pin
}

void OWInit(void)
{
  OW_CR1 |= OW_BUSMASK;  // push-pull output
  OW_CR2 &= ~OW_BUSMASK;  // output speed limited to 10 MHz
  DelayInit(); // timer4 initialization
}

char OWReset(void)
{
    DISABLE_INTERRUPTS // set interrupt mask
  
    char cPresence; // If there are any slaves ready to operate this variable 
                    // show it
  
    OWOutputMode(); // select output mod
    OWLow(); // pull the 1-wire bus low
    Delay_us(OW_RESET_PULSE); //reset pulse
  
    OWInputMode(); // releases bus
    Delay_us(OW_WAIT_PRESENCE_PULSE); 
                // wait until pull-up resistor pulls the 1-wire bus high and 
                // until device will detected this rising. When device detects 
                // this rising edge, it wait 15-60us and then transmits a 
                // presence pulse by pulling the 1-wire bus low for 60-240us.
    cPresence = (OW_IDR & OW_BUSMASK); // if there are any slave(s) on the bus
                    // and are ready to operate this variable is equal "0".
                    // Else this variable is equal any positive number.
    Delay_us(OW_PRESENCE_PULSE); // presence pulse
  
    ENABLE_INTERRUPTS // reset interrupt mask
    if(!cPresence) return ANY_DEVICE_IS_PRESENT; // if there are any slave devices
                                               // on the bus return "ANY_DEVICE_IS_PRESENT" 
    else return NO_DEVICE;
}

void OWWriteByte(char cByte)
{
    DISABLE_INTERRUPTS // set interrupt mask
  
    char cCounter = ZERO_INIT;
    while(BITS_IN_BYTE > cCounter) 
    { // in this loop the master write all eight bits
        OWOutputMode(); // select output mod
        OWLow(); // pull the 1-wire bus low
        Delay_1us(OW_READ_WRITE_TIME_SLOT_INIT); // the master initiates write time 
                                                // slot
    
        if(cByte & 0x01) OWInputMode(); // if the following bit is "1" master 
                                        // write "1" (leave the bus high)
        Delay_us(OW_READ_WRITE_TIME_SLOT); // during this time device is reading the 
                                       // state of the 1-wire bus
        OWInputMode(); // releases bus
        cByte >>= 1;
        cCounter++;
        Delay_1us(OW_RECOVERY_TIME); // recovery time between slots
    }
  
    ENABLE_INTERRUPTS // reset interrupt mask
}

char OWReadByte(void)
{
    DISABLE_INTERRUPTS // set interrupt mask
  
    char cCounter = ZERO_INIT, cByte = ZERO_INIT;
    while(BITS_IN_BYTE > cCounter) 
    { // in this loop the master read all eight bits
        OWOutputMode(); // select output mod
        OWLow(); // pull the 1-wire bus low
        Delay_1us(OW_READ_WRITE_TIME_SLOT_INIT); // the master issues read time
                                                // slot
    
        OWInputMode(); // releases bus
        Delay_us(OW_WAIT_VALID_DATA); // wait until the device transmit the next bit
        if(OW_IDR & OW_BUSMASK) cByte |= 1 << cCounter; // the master read the next
                                                        // bit
        Delay_us(OW_END_OF_READ_TIME_SLOT); // wait the end of read time slot
        cCounter++;
        Delay_1us(OW_RECOVERY_TIME); // recovery time between slots
    }
  
    ENABLE_INTERRUPTS; // reset interrupt mask
    return cByte;
}

char OWReadROM(char *pcAddress)
{
    DISABLE_INTERRUPTS // set interrupt mask
  
    char cPresence = OWReset(); // if there are any devices on the bus cPresence 
                                // is equal "0"
    if(ANY_DEVICE_IS_PRESENT == cPresence) 
    {
        OWWriteByte(OW_CMD_READROM); // transmit "Read ROM" command
        for(char cCounter = 0; cCounter < BITS_IN_BYTE; cCounter++)
        pcAddress[cCounter] = OWReadByte(); // read address bytes
    
        ENABLE_INTERRUPTS // reset interrupt mask
        return ANY_DEVICE_IS_PRESENT;
    };
  
    ENABLE_INTERRUPTS // reset interrupt mask
    return NO_DEVICE;
}

char OWMatchROM(char *pcAddress)
{
    DISABLE_INTERRUPTS // set interrupt mask
  
    char cPresence = OWReset(); // if there are any devices on the bus 
                              // cPresence is equal "0"
    if(ANY_DEVICE_IS_PRESENT == cPresence) 
    {
        OWWriteByte(OW_CMD_MATCHROM); // transmit "Match ROM" command
        for(char cCounter = 0; cCounter < BITS_IN_BYTE; cCounter++)
            OWWriteByte(pcAddress[cCounter]); // write address bytes
      
        ENABLE_INTERRUPTS // reset interrupt mask
        return ANY_DEVICE_IS_PRESENT;
    };
  
    ENABLE_INTERRUPTS // reset interrupt mask
    return NO_DEVICE;
}

char OWSkipROM(void)
{
    DISABLE_INTERRUPTS // set interrupt mask
  
    char cPresence = OWReset(); // if there  are any devices on the bus 
                                // cPresence is equal "0"
    if(ANY_DEVICE_IS_PRESENT == cPresence) 
    {
        OWWriteByte(OW_CMD_SKIPROM); // the master is addresses to all devices
    
        ENABLE_INTERRUPTS // reset interrupt mask
        return ANY_DEVICE_IS_PRESENT;
    };
  
    ENABLE_INTERRUPTS // reset interrupt mask
    return NO_DEVICE;
}

