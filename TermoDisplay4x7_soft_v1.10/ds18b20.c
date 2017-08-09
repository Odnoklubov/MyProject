#include "ds18b20.h"

//////////////////////////////////////////
// AT TODO: find way work without global variable(read therm mode from device, or get input function parameter)
//unsigned short siWaitMs; // contain time that need to convert temperature
//////////////////////////////////////////

char ThermInitAll(char cThermMode)
{
    DISABLE_INTERRUPTS
    OWInit();
    char cCommandResult = OWSkipROM(); // The master addresses to all devices. If 
                                        // there are any devices on the bus 
                                        // cCommandResult is equal "0"
    if(NO_ERROR == cCommandResult) 
    {
        OWWriteByte(THERM_CMD_WRITESCRATCHPAD);
        OWWriteByte(THERM_USER_BYTE_0); // write UserByte0
        OWWriteByte(THERM_USER_BYTE_1); // write UserByte1 
        OWWriteByte(cThermMode);
        // AT TODO: find way work without global variable(read therm mode from device, or get input function parameter)
//        switch(cThermMode)
//        { // choose time that needed to conversion
//            case THERM_MODE_9BIT: 
//                siWaitMs = THERM_TIME_9BIT; 
//                break;
//            case THERM_MODE_10BIT: 
//                siWaitMs = THERM_TIME_10BIT; 
//                break;
//            case THERM_MODE_11BIT: 
//                siWaitMs = THERM_TIME_11BIT; 
//                break;
//            case THERM_MODE_12BIT: 
//            siWaitMs = THERM_TIME_12BIT; 
//            break;
//        };
    }
  
    ENABLE_INTERRUPTS
    return cCommandResult;
}

//char ThermInitAll(char cThermMode)
//{
//  DISABLE_INTERRUPTS
//  
//  char cCommandResult = OWSkipROM();
//  if(!cCommandResult) {
//    OWWriteByte(THERM_CMD_WRITESCRATCHPAD);
//    OWWriteByte(0);
//    OWWriteByte(0);
//    OWWriteByte(cThermMode);
//  }
//  
//  ENABLE_INTERRUPTS
//  return cCommandResult;
//}

//char ThermInit(char *pcAddress, char cThermMode)
//{
//  return ThermWriteScratchpad(pcAddress,0,0,cThermMode);
//}

char ThermConvertTemp(void)
{
    DISABLE_INTERRUPTS
  
    char cCommandResult = OWSkipROM(); // The master addresses to all devices. If 
                                        // there are any devices on the bus 
                                        // cCommandResult is equal "0"
    if(NO_ERROR == cCommandResult) 
    {
        OWWriteByte(THERM_CMD_CONVERTTEMP); // send command "Convert temperature"
    
        // AT TODO: find way work without global variable(read therm mode from device, or get input function parameter)
        Delay_msSafe(THERM_TIME_11BIT);
        //Delay_ms(siWaitMs); // wait until device convert temperature
        OWInputMode(); // releases bus
    }
  
    ENABLE_INTERRUPTS
    return cCommandResult;
}

//char ThermWriteScratchpad(char *pcAddress,char cUserByte1,char cUserByte2,
//                          char cThermMode)
//{
//  DISABLE_INTERRUPTS
//  
//  char cCommandResult = OWMatchROM(pcAddress);
//  if(!cCommandResult) {
//    OWWriteByte(THERM_CMD_WRITESCRATCHPAD);
//    OWWriteByte(cUserByte1);
//    OWWriteByte(cUserByte2);
//    OWWriteByte(cThermMode);
//  }
//  
//  ENABLE_INTERRUPTS
//  return cCommandResult;
//}

char ThermReadScratchpad(unsigned char *aucScratchpad)
{
    DISABLE_INTERRUPTS
  
    char cCommandResult = OWSkipROM(); // The master addresses to all devices. If 
                                        // there are any devices on the bus 
                                        // cCommandResult is equal "0"
    if(NO_ERROR == cCommandResult) 
    {
        OWWriteByte(THERM_CMD_READSCRATCHPAD); // send command "Read Scratchpad"
    
        // read bytes of scratchpad
        for(char cCounter = 0; cCounter < QUANTITY_OF_SCRATCHPAD_BYTES; cCounter++) 
            aucScratchpad[cCounter] = OWReadByte();
    }
  
    ENABLE_INTERRUPTS
    return cCommandResult;
}

char ThermReadTemperature(unsigned char *aucTemperature)
{
    DISABLE_INTERRUPTS
  
    char cCommandResult = OWSkipROM(); // The master addresses to all devices. If 
                                        // there are any devices on the bus 
                                        // cCommandResult is equal "0"
    unsigned char aucScratchpad[QUANTITY_OF_SCRATCHPAD_BYTES];
    aucTemperature[0] = INIT_ZERO;
    aucTemperature[1] = INIT_ZERO;
    aucTemperature[2] = INIT_ZERO;
    //unsigned short int usiTemp  = INIT_ZERO; // this variable contains fractional part
  
    if(NO_ERROR == cCommandResult) 
    {
        OWWriteByte(THERM_CMD_READSCRATCHPAD); // send command "Read Scratchpad"
    
        // read scratchpad
        for(char cCounter = 0; cCounter < QUANTITY_OF_SCRATCHPAD_BYTES; cCounter++)                                         
            aucScratchpad[cCounter] = OWReadByte();
        
        aucTemperature[0] = aucScratchpad[0];
        aucTemperature[1] = aucScratchpad[1];
    }
  
    ENABLE_INTERRUPTS
    return cCommandResult;
}

float GetTemperature()
{
    uint8 aucTemperature[4];
    
    TIM3_IER_UIE = 0; // disable RTC interrupt
    char tempFlag = ThermConvertTemp();
    tempFlag = ThermReadTemperature(aucTemperature);
    TIM3_IER_UIE = 1; // enable RTC interrupt
    
    INT32 celsiumTempValue = 0;
    for(uint8 i = 0; i < 0x08; i++)
    {
      if(aucTemperature[0] & 0x01 == 0x01)
        celsiumTempValue += 0x01 << i;
      aucTemperature[0] >>= 0x01;
    }
    
    for(uint8 i = 8; i < 16; i++)
    {
      if(aucTemperature[1] & 0x01 == 0x01)
        celsiumTempValue += 0x01 << i;
      aucTemperature[1] >>= 0x01;
    }
    float curTemp = celsiumTempValue * 0.0625;
    return curTemp;
}
INT16 GetCommonTempValue()
{//  Gets INT16 value of temperature = 10*GetTemperature();
    float fTemperatureValue = 0;        //  Float temperature value
    INT16 sIntTempValue;                //  Integer part of temperature value
    INT16 sDecTempValue;                //  Decimal part of temperature value
  
    fTemperatureValue = GetTemperature();
    Delay_msSafe(100);
    fTemperatureValue = GetTemperature();
    
    sIntTempValue = (INT16)fTemperatureValue;   
    sDecTempValue = (INT16)((fTemperatureValue - sIntTempValue)*100);
    
    if(sDecTempValue % 10 < 5)
    {
        sDecTempValue /= 10;
    }
    else
    {
        sDecTempValue /= 10;
        sDecTempValue++;
    }
    
    return sIntTempValue*10 + sDecTempValue;
}