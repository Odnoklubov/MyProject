#ifndef _RFM_22_
#define _RFM_22_
#include "types.h"
#define STANDBY_MODE  0x00
#define READY_MODE    0x01   //для уменьшения токопотребления см стр 17. 0x62
#define TUNE_MODE     0x02
#define RECIEVE_MODE  0x04
#define TRANSMIT_MODE 0x08
#define SLEEP_MODE    0x20    //ошибка на стр 17
#define SENSOR_MODE   0x40

//#define NULL   0x00
#define TX_ANT 0x01
#define RX_ANT 0x02

#define RADIO_MAX_PACKET_SIZE 64

#define SDN_LOW                 PC_ODR_ODR2 = 0
#define SDN_HIGH                PC_ODR_ODR2 = 1

#define Event_RFM (PC_IDR & MASK_PC_IDR_IDR3)

#define POWER_20dB      0x07            //  100mW
#define POWER_17dB      0x06            //  50mW
#define POWER_14dB      0x05            //  25mW
#define POWER_11dB      0x04            //  12.5mW
#define POWER_8dB       0x03            //  6.3mW
#define POWER_5dB       0x02            //  3.2mW
#define POWER_2dB       0x01            //  1.6mW
#define POWER_1dB      0x00            //  1.3mW




#define CHECK_HEADER_NO           0x00
#define CHECK_HEADER_0            0x01  
#define CHECK_HEADER_1            0x02
#define CHECK_HEADER_2            0x04
#define CHECK_HEADER_3            0x08
#define CHECK_HEADER_ALL            0x0f
#define CHECK_BROAD_NO          0x00
#define CHECK_BROAD_0           0x10
#define CHECK_BROAD_1           0x20
#define CHECK_BROAD_2           0x40
#define CHECK_BROAD_3           0x80
#define CHECK_BROAD_ALL           0xf0

#define SYNC3         0x00      
#define SYNC32        0x02
#define SYNC321       0x04
#define SYNC3210      0x06
#define HEAD3         0x10
#define HEAD32        0x20   
#define HEAD321       0x30
#define HEAD3210      0x40
#define NOHEAD        0x00


#define IFFERROR                0x80
#define ITXFFAFULL              0x40
#define ITXFFAEM                0x20
#define IRXFFAFULL              0x10
#define IEXT                    0x08
#define IPKSENT                 0x04
#define IPKVALID                0x02
#define ICRCERROR               0x01

#define ISWDET                  0x80
#define IPREAVAL                0x40
#define IPREAINVAL              0x20
#define IRSSI                   0x10
#define IWUT                    0x08
#define ILBD                    0x04
#define ICHIPRDY                0x02
#define IPOR                    0x01

#define BROAD           0xff
#define ZERO            0x00
// Тестовые изменения

#define DEVICE_TYPE_R                                 0x00
#define DEVICE_VERSION_R                              0x01
#define DEVICE_STATUS_R                               0x02
#define INTERRUPT_STATUS_1_R                          0x03
#define INTERRUPT_STATUS_2_R                          0x04
#define INTERRUPT_ENABLE_1_RW                         0x05
#define INTERRUPT_ENABLE_2_RW                         0x06
#define OPERATION_FUNCTION_CNTRL_1_RW                   0x07
#define OPERATION_FUNCTION_CNTRL_2_RW                   0x08
#define CRYSTALL_OSCILLATOR_LOAD_CAP_RW                 0x09
#define MC_OUTPUT_CLOCK_RW                              0x0A
#define GPIO_0_CONFIGURATION_RW                         0x0B
#define GPIO_1_CONFIGURATION_RW                         0x0C
#define GPIO_2_CONFIGURATION_RW                         0x0D
#define IO_PORT_CONFIGURATION_RW                        0x0e
#define ADC_CONFIGURATION_RW                            0x0f
#define ADC_SENSOR_AMPLIFIER_OFFSET_RW                  0x10
#define ADC_VALUE_R                                   0x11
#define TEMPERATURE_SENSOR_CONTROL_RW                   0x12
#define TEMPERATURE_VALUE_OFFSET_RW                     0x13
#define WAKEUP_TIMER_PERIOD_1_RW                        0x14
#define WAKEUP_TIMER_PERIOD_2_RW                        0x15
#define WAKEUP_TIMER_PERIOD_3_RW                        0x16
#define WAKEUP_TIMER_VALUE_1_R                          0x17
#define WAKEUP_TIMER_VALUE_2_R                          0x18
#define LOW_DUTY_CYCLE_DURATION_RW                      0x19
#define LOW_BATTERY_DETECTOR_THRESHOLD_RW               0x1A
#define BATTERY_VOLTAGE_LEVEL_R                         0x1B
#define IF_FILTER_BANDWITH_RW                           0x1C
#define AFC_LOOP_GEARSHIFT_OVERRIDE_RW                  0x1D
#define AFC_TIMING_CONTROL_RW                           0x1E
#define CLOCK_RECOVERY_GEARSHIFT                        0x1F
#define CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW            0x20
#define CLOCK_RECOVERY_OFFSET_2_RW                      0x21
#define CLOCK_RECOVERY_OFFSET_1_RW                      0x22
#define CLOCK_RECOVERY_OFFSET_0_RW                      0x23
#define CLOCK_RECOVERY_TIMING_LOOP_1_RW                 0x24
#define CLOCK_RECOVERY_TIMING_LOOP_0_RW                 0x25
#define RECEIVED_SIGNAL_STRENGTH_INDICATOR_R            0x26
#define RSSI_THRESHOLD_CLEAR_INDICATOR_RW               0x27
#define ANTENNA_DIVERSITY_REGISTER_1_R                  0x28
#define ANTENNA_DIVERSITY_REGISTER_2_R                  0x29

#define AFC_LIMITER_RW                                  0x2a
#define AFC_CORRECTION_READ_R                           0x2b
#define OOK_COUNTER_VALUE_1_RW                          0x2c
#define OOK_COUNTER_VALUE_2_RW                          0x2d
#define SLICER_PEAK_HOLD_RW                             0x2e

#define DATA_ACCESS_CONTROL_RW                          0x30
#define EZMAC_RW                                        0x31
#define HEADER_CONTROL_1_RW                             0x32
#define HEADER_CONTROL_2_RW                             0x33
#define PREAMBLE_LENGTH_RW                              0x34
#define PREAMBLE_DETECTION_CONTROL_RW                   0x35
#define SYNC_WORD_3_RW                                  0x36
#define SYNC_WORD_2_RW                                  0x37
#define SYNC_WORD_1_RW                                  0x38
#define SYNC_WORD_0_RW                                  0x39
#define TRANSMIT_HEADER_3_RW                            0x3a
#define TRANSMIT_HEADER_2_RW                            0x3b
#define TRANSMIT_HEADER_1_RW                            0x3c
#define TRANSMIT_HEADER_0_RW                            0x3d
#define TRANSMIT_PACKET_LENGTH_RW                       0x3e
#define CHECK_HEADER_3_RW                               0x3f
#define CHECK_HEADER_2_RW                               0x40
#define CHECK_HEADER_1_RW                               0x41
#define CHECK_HEADER_0_RW                               0x42
#define HEADER_ENABLE_3_RW                              0x43
#define HEADER_ENABLE_2_RW                              0x44 
#define HEADER_ENABLE_1_RW                              0x45
#define HEADER_ENABLE_0_RW                              0x46
#define RECEIVED_HEADER_3_R                           0x47
#define RECEIVED_HEADER_2_R                           0x48
#define RECEIVED_HEADER_1_R                           0x49
#define RECEIVED_HEADER_0_R                           0x4a
#define RECEIVED_PACKET_LENGTH_R                      0x4b


#define ADC8_CONTROL_RW                                 0x4F 

#define ANALOG_TEST_BUS_RW                              0x50
#define DIGITAL_TEST_BUS_RW                             0x51
#define TX_RAMP_CONTROL_RW                              0x52
#define PLL_TUN_TIME_RW                                 0x53
#define RESERVED                                        0x54
#define CALIBRATION_CONTROL_RW                          0x55
#define MODEM_TEST_RW                                   0x56
#define CHARGE_PUMP_TEST_RW                             0x57
#define CHARGEPUMP_CURRENT_TRIM_OVER_RW                 0x58
#define DIVIDER_CURRENT_TRIMMING_RW                     0x59
#define VCO_CURRENT_TRIMMING_RW                         0x5a
#define VCO_CALIBRATION_OVERRIDE _RW                    0x5b
#define SYNTHESIZER_TEST_RW                             0x5c
#define BLOCK_ENABLE_OVERRIDE_1_RW                      0x5d
#define BLOCK_ENABLE_OVERRIDE_2_RW                      0x5e
#define BLOCK_ENABLE_OVERRIDE_3_RW                      0x5f


#define CHANNEL_FILTER_COEFFICIENT_ADDRESS_RW           0x60

#define CHANNEL_FILTER_COEFFICIENT_VALUE_RW             0x61

#define CRYSTAL_OSCILLATORCONTROL_TEST_RW               0x62


#define RC_OSCILLATOR_CALIBRATION_COARSE_RW             0x63
#define RC_OSCILLATOR_CALIBRATION_FINE_RW               0x64
#define LDO_CONTROL_OVERRIDE_RW                         0x65
#define LDO_LEVEL_SETTING_RW                            0x66
#define DELTA_SIGMA_ADC_TUNE_1_RW                       0x67
#define DELTA_SIGMA_ADC_TUNE_2_RW                       0x68


#define AGC_OVERRIDE_1_RW                               0x69
#define AGC_OVERRIDE_2_RW                               0x6a

#define TX_POWER_RW                                     0x6d
#define TX_DATA_RATE_1_RW                               0x6e
#define TX_DATA_RATE_0_RW                               0x6f
#define MODULATION_MODE_CONTROL_1_RW                    0x70
#define MODULATION_MODE_CONTROL_2_RW                    0x71
#define FREQUENCY_DEVIATION_RW                          0x72
#define FREQUENCY_OFFSET_1_RW                           0x73
#define FREQUENCY_OFFSET_2_RW                           0x74
#define FREQUENCY_BAND_SELEC_RW                         0x75
#define NOMINAL_CARRIER_FREQUENCY_1_RW                  0x76
#define NOMINAL_CARRIER_FREQUENCY_0_RW                  0x77
#define FREQUENCY_HOPPING_CHANNEL_RW                    0x79
#define FREQUENCY_HOPPING_STEP_SIZE_RW                  0x7a
#define TX_FIFO_CONTROL_1_RW                            0x7c
#define TX_FIFO_CONTROL_2_RW                            0x7d
#define RX_FIFO_CONTROL_RW                              0x7e
#define FIFO_ACCESS_RW                                  0x7f

struct RadioPacket
{
    uint8 aRadioPacket[RADIO_MAX_PACKET_SIZE];
    uint8 length;
    bool status;
};

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Initialize RFM22 module
//!
//! \param void
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////

void Rfm22Init(void);      
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Enable interrupt on receive packet
//!
//! \param void
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void RFM_interrupt_RX (void);	
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Enable interrupt on end of transmit packet
//!
//! \param void
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void RFM_interrupt_TX (void);		
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Transmitting of array data from FIFO to Radiochanell
//!
//! \param size - the number of bytes in the transmitted packet 
//! \param data - the data array to transmit. Max length of array 64 bytes
//!
//! \return 
//!
////////////////////////////////////////////////////////////////////////////////
void SendPacket (uint8* data,uint8 size);	
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Transmitting of array data from FIFO to Radiochanell
//!
//! \param start - first byte of data to transmitt		
//! \param start - last byte of data to transmitt
//! \param data - the data array to transmit. Max length of array 64 bytes
//!
//! \return 
//!
////////////////////////////////////////////////////////////////////////////////
//void SendPackeT(uint8* data,uint8 start, uint8 end);
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Transfer data from FIFO to data array
//!
//! \param data - target array
//!
//! \return length of recieve array of bytes
//!
////////////////////////////////////////////////////////////////////////////////
uint8 ReceivePacket (uint8* data);    
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Waiting of transmit packet interrupt
//!
//! \param void
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void TX_ON (void);	
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Waiting of recieve packet interrupt
//!
//! \param void
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void RX_ON (void);	
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Waiting of recieve packet interrupt with time out 
//!
//! \param void
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void RX_ON_TIMEOUT (uint16 nTime);
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Turn Radiomodule to Ready mode for faster starting digital modem
//!
//! \param void
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void READY_ON (void);	
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Sending series from 1 to 254. Function for testing purposes
//!
//! \param void
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void TestTransmitting(void);
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Reciev function for testing purposes
//!
//! \param void
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void TestRecieving(void);
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Recieve RSSI
//!
//! \param void
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void MeasuringRSSI(void);

// The need for these features appeared after I discovered that radio allows redispatches last downloaded package without restarting it in the FIFO.
// And it is useful to us
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Write to TX_FIFO an array aData of size bytes
//!
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void WriteToFIFO(uint8* aData,uint8 size);
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Read an array of size bytes from RX_FIFO to aData 
//!
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void ReadFromFIFO(uint8* aData,uint8 size);

void Rfm22TerminalInit();
void Rfm22NodeInit();

void RfmSetup868Dev30();
void RfmSetup868Dev15Manchester();
void RfmSetup868Dev40Manchester();
void RfmSetup433Dev15Manchester();
void RfmSetup433Dev15White();
void RfmSetup433Dev75();
void RfmSetup433Dev50();
void RfmSetup433Dev30();
void RfmSetup433Dev15();
void RfmSetup868Dev15();

void RfmSetup433Dev5();
void RfmSetup433Dev15_64kRate();
void RfmSetup433Dev15_1kRate();
void RfmSetup433Dev15_4kRate();
void RfmSetup433Dev15_8kRate();
void RfmSetup433Dev15_433MHz();
void RfmSetup433Dev15_420MHz();
void RfmSetup433Dev15_450MHz();
void RfmSetup433Dev5Manchester();
void RfmSetup433Dev30Manchester();
void RfmSetup433Dev50Manchester();
void RfmSetup433Dev100Manchester();
void RfmSetup433Dev15Manchester_64kRate();
void RfmSetup433Dev15Manchester_256kRate();
void RfmSetup433Dev15Manchester_4kRate();
void RfmSetup433Dev15Manchester_8kRate();
void RfmSetup433Dev15Manchester_433MHz();
void RfmSetup433Dev15Manchester_420MHz();
void RfmSetup433Dev15Manchester_450MHz();
#endif /* _RFM_22_*/