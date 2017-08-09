////////////////////////////////////////////////////////////////////////////////
//!
//! \file ds18b20.h
//!
//! \brief Module for ds18b20 functions.
//! The ds18b20 must powered by an external supply.
//!
//! \author Dmitriy Vorobyov "dima@mcsys.by"
//!
////////////////////////////////////////////////////////////////////////////////

#ifndef DS18B20
#define DS18B20

#include "one-wire.h"

////////////////////////////////////////////////////////////////////////////////
// Macro definitions
////////////////////////////////////////////////////////////////////////////////

#define QUANTITY_OF_SCRATCHPAD_BYTES    0x09
#define NO_ERROR                        0x01

// Thermometer commands
#define THERM_CMD_CONVERTTEMP           0x44
#define THERM_CMD_WRITESCRATCHPAD       0x4E
#define THERM_CMD_READSCRATCHPAD        0xBE
#define THERM_CMD_COPYSCRATCHPAD        0x48
#define THERM_CMD_RECALLE               0xB8
#define THERM_CMD_READPOWERSUPPLY       0xB4

// Thermometer mode
#define THERM_MODE_9BIT                 0x1F // the resolution of the 
                                             // temperature  sensor is 0.5C
#define THERM_MODE_10BIT                0x3F // the resolution of the 
                                             // temperature  sensor is 0.25C
#define THERM_MODE_11BIT                0x5F // the resolution of the 
                                             // temperature  sensor is 0.125C
#define THERM_MODE_12BIT                0x7F // the resolution of the 
                                             // temperature  sensor is 0.0625C

// Thermometer conversion time(ms)
#define THERM_TIME_9BIT                 100
#define THERM_TIME_10BIT                200
#define THERM_TIME_11BIT                400
#define THERM_TIME_12BIT                800

// Thermometer accuracy bits adds
#define THERM_ACCURACY_05               0x1388
#define THERM_ACCURACY_025              0x09C4
#define THERM_ACCURACY_0125             0x04E2
#define THERM_ACCURACY_00625            0x0271

// Thermometer initialization user bytes
#define THERM_USER_BYTE_0               0x05
#define THERM_USER_BYTE_1               0x50

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Thermometer functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! 
//! \brief The initialization of  all thermometers.
//!
//! \param cThermMode - Thermometer mode. This variable is chooses the
//! resolution of temperature sensor. See modes in macro definition section.
//!
//! \return If there are any devices on the bus return "0".
//!
////////////////////////////////////////////////////////////////////////////////
char ThermInitAll(char cThermMode);

////////////////////////////////////////////////////////////////////////////////
//! 
//! \brief The initialization of individual thermometer.
//!
////////////////////////////////////////////////////////////////////////////////
//char ThermInit(char cThermMode, char *pcAddress);

////////////////////////////////////////////////////////////////////////////////
//! 
//! \brief Initiate a single temperature conversion.
//!
//! \return If there are any devices on the bus return "0".
//!
////////////////////////////////////////////////////////////////////////////////
char ThermConvertTemp(void);

////////////////////////////////////////////////////////////////////////////////
//! 
//! \brief This function allows to read the contents of the scratchpad. The data
//! transfer starts with the least significant bit of  byte 0 and continues 
//! through the scratchpad until the 9th byte is read.
//!
//! \param pcScratchpad - contains bytes of the scratchpad
//! byte 0 - Temperature LSB (50h)
//! byte 1 - Temperature MSB (05h)
//! byte 2 - TH Register or User Byte 1
//! byte 3 - TL Register or User Byte 2
//! byte 4 - Configuration Register
//! byte 5 - Reserved (FFh)
//! byte 6 - Reserved
//! byte 7 - Reserved
//! byte 8 - CRC
//!
//! \return If there are any devices on the bus return "0".
//!
////////////////////////////////////////////////////////////////////////////////
char ThermReadScratchpad(unsigned char *aucScratchpad);

////////////////////////////////////////////////////////////////////////////////
//! 
//! In plans
//!
////////////////////////////////////////////////////////////////////////////////
//char ThermWriteScratchpad(char *pcAddress,char cUserByte1,char cUserByte2,
//                           char cThermMode);
//void ThermCopyScratchpad(void);
//void ThermRecallE(void);
//void ThermReadPowerSupply(void;

////////////////////////////////////////////////////////////////////////////////
//! 
//! \brief This function allows to read temperature. Call this function after 
//! calling ThermConvertTemp function.
//!
//! \param aucTemperature - contains two bytes with temperature
//! byte 0:
//! bites: 7    6    5    4    3     2     1     0
//!       2^3  2^2  2^1  2^0  2^-1  2^-2  2^-3  2^-4
//! byte 1:
//! bites: 7   6   5   4   3    2    1    0
//!        s   s   s   s   s   2^6  2^5  2^4
//!
//! \return If there are any devices on the bus return "0".
//!
////////////////////////////////////////////////////////////////////////////////
char ThermReadTemperature(unsigned char *aucTemperature);
float GetTemperature();
INT16 GetCommonTempValue();
#endif //DS18B20