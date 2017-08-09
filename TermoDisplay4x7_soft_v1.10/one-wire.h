////////////////////////////////////////////////////////////////////////////////
//!
//! \file one-wire.h
//!
//! \brief Module for 1-Wire interface, base level.
//! The transfer sequencer is as follows:
//! 1. ROM command (followed by any required data exchange)
//! 2. Function command (followed by any required data exchange)
//!
//! \author Dmitriy Vorobyov "dima@mcsys.by"
//!
////////////////////////////////////////////////////////////////////////////////

#ifndef OW_BASE_LEVEL
#define OW_BASE_LEVEL

#include "types.h"
#include "iostm8s105c6.h"
#include "delay.h"

////////////////////////////////////////////////////////////////////////////////
// Macro definitions
////////////////////////////////////////////////////////////////////////////////

#define NO_DEVICE                       0
#define ANY_DEVICE_IS_PRESENT           1

// 1-wire port definitions
#define F_CPU_MHz                       16
#define OW_DDR                          PG_DDR
#define OW_ODR                          PG_ODR
#define OW_IDR                          PG_IDR
#define OW_CR1                          PG_CR1
#define OW_CR2                          PG_CR2
#define OW_BUSMASK                      0x01

// 1-wire commands
#define OW_CMD_SEARCHROM                0xF0
#define OW_CMD_READROM                  0x33
#define OW_CMD_MATCHROM                 0x55
#define OW_CMD_SKIPROM                  0xCC
#define OW_CMD_ALARMSEARCH              0xEC

// other definitions
#define ZERO_INIT                       0x00
#define FF_INIT                         0xff
#define OW_RESET_PULSE                  240//480
#define OW_WAIT_PRESENCE_PULSE          35//70
#define OW_PRESENCE_PULSE               205//410
#define OW_READ_WRITE_TIME_SLOT_INIT    1//2
#define OW_READ_WRITE_TIME_SLOT         30//60
#define OW_RECOVERY_TIME                1//2
#define OW_WAIT_VALID_DATA              8//16
#define OW_END_OF_READ_TIME_SLOT        23//46

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! 
//! \brief Drive OW_ODR high
//!
////////////////////////////////////////////////////////////////////////////////
void OWHigh(void);

////////////////////////////////////////////////////////////////////////////////
//! 
//! \brief drive OW_ODR low
//!
////////////////////////////////////////////////////////////////////////////////
void OWLow(void);

////////////////////////////////////////////////////////////////////////////////
//! 
//! \brief Select input mod for the 1-Wire pin
//!
////////////////////////////////////////////////////////////////////////////////
void OWInputMode(void);

////////////////////////////////////////////////////////////////////////////////
//! 
//! \brief Select output mod for the 1-Wire pin
//!
////////////////////////////////////////////////////////////////////////////////
void OWOutputMode(void);

////////////////////////////////////////////////////////////////////////////////
//! 
//! \brief The initialization of 1-Wire interface. It is necessary to call this 
//! function before calling other functions or before working with the connected 
//! devices.
//!
////////////////////////////////////////////////////////////////////////////////
void OWInit(void);

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Write byte. Byte will be transmitted least significant bit first.
//!
//! \param cByte - The byte that will be written.
//!
////////////////////////////////////////////////////////////////////////////////
void OWWriteByte(char cByte);

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Read byte. Byte will be transferred least significant bit first.
//!
//! \return The byte that was read.
//!
////////////////////////////////////////////////////////////////////////////////
char OWReadByte(void);

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Initialization sequence. 
//! All transactions on the 1-Wire bus begins with an initialization 
//! sequence.
//! The initialization sequence consists of a reset pulse transmitted by the bus
//! master followed by presence pulse(s) transmitted by the slave(s). 
//! The presence pulse lets the bus master know that slave devices are on the
//! bus and are ready to operate.
//!
//! \return If there are any slave devices are on the bus return 
//! "ANY_DEVICE_IS_PRESENT". Else return "NO_DEVICE".
//!
////////////////////////////////////////////////////////////////////////////////
char OWReset(void);

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Read ROM Command.
//! This command can only be used when there is one slave on the bus. It allows
//! the bus master to read the slave's 64-bit ROM code without using the Search
//! ROM procedure
//!
//! \param pcAddress - Slave's 8-byte ROM code (the slave's address).
//!
//! \return If there are any slave devices are on the bus return 
//! "ANY_DEVICE_IS_PRESENT". Else return "NO_DEVICE".
//!
////////////////////////////////////////////////////////////////////////////////
char OWReadROM(char *pcAddress);

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Match ROM Command.
//! The match ROM command followed by a 8-byte ROM code sequence allows the bus
//! master to address a specific slave device. Only the slave that exactly
//! matches the 8-byte ROM code sequencer will respond to the function command
//! issued by the master.
//!
//! \param pcAddress - Slave's 8-byte ROM cod (the slave's address).
//!
//! \return If there are any slave devices are on the bus return 
//! "ANY_DEVICE_IS_PRESENT". Else return "NO_DEVICE".
//!
////////////////////////////////////////////////////////////////////////////////
char OWMatchROM(char *pcAddress); // this function is needed to be check

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Skip ROM command.
//! The master can use this command to address all devices on the bus
//! simultaneously without sending out any ROM code information.
//!
//! \return If there are any slave devices are on the bus return 
//! "ANY_DEVICE_IS_PRESENT". Else return "NO_DEVICE".
//!
////////////////////////////////////////////////////////////////////////////////
char OWSkipROM(void);

////////////////////////////////////////////////////////////////////////////////
// In Plans
////////////////////////////////////////////////////////////////////////////////
//void OWSearchROM(char *pcAddress);
//void OWAlarmSearch
//void OWCRC(char *pcArray, char n);

#endif //OW_BASE_LEVEL