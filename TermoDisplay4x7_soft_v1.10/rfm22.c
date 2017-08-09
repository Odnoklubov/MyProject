#include "spi.h"
#include "rfm22.h"
#include "LEDs.h"
#include "types.h"

#ifndef _DELAY_H_
#include "delay.h"
#endif

#define FREQ_433                                                                //  Frequency of RFM22B 433 MHz or 868 MHz (not 433 MHz)


extern uint8 aTxPacket[64];
extern uint8 aRxPacket[64];   
struct RadioPacket RxRadioPacket={'\0',0,FALSE};
struct RadioPacket TxRadioPacket={'\0',0,FALSE};
extern bool timeout_flag;


//Global variable for definition of count bytes of data  in the transmitted/received package. 
//It can be vary in app
//It should be less than or equal 64
volatile uint8 DATA_PACKET_LENGTH; 

//Setting rfm22. Tuning parameters are generated RF22B 23B 31B 42B 43B Register Settings_RevB1-v5.xls 

void Rfm22Init()
{ 
  // Setting IRQ port
  PC_DDR&=~MASK_PC_DDR_DDR3;            //as input
  PC_CR1&=~MASK_PC_CR1_C13;             //as floating input
  PC_CR2&=~MASK_PC_CR2_C23;             // external interrupt disable
  //Setting SDN to LOW
  PC_CR1 |=  MASK_PC_CR1_C12;           // push-pull output
  PC_CR2 &= ~MASK_PC_CR2_C22;           // low speed output
  PC_DDR |=  MASK_PC_DDR_DDR2;          // as output
  SDN_HIGH;                             // shutdown module. So, all registers will be back to default
  Delay_ms(5);
  SDN_LOW;
  
  Delay_ms(50);                                                                 // Delay need to exit from shutdown mode
  
#ifdef FREQ_433
  RfmSetup433Dev5();   //for 434,5 MHz without Manchester without data whitening f deviation=15khz sens -117db 2kbps
#else
  RfmSetup868Dev30();    //for 868 MHz without Manchester without data whitening f deviation=30khz sens -117db 2kbps
#endif
  
  //-------------Power:
  SPITransmitByte(TX_POWER_RW,0x08|POWER_8dB);                                 //0x08 to disable lna in TX mode
  //-------------Preamble length:
  // 8bit - 0x08, 32bit - 0x20, 40bit - 0x28, 64bit - 0x40, 128bit - 0x80, 255bit - 0xFF
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x40);                          //  40bit - recom1

  // 5bit - 0x28, 8bit - 0x40, 20bit - 0xA0, 31bit - 0xF8
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0xA0);        //  PrDetTreshold = 20bit - recom1
  
  SPITransmitByte(AGC_OVERRIDE_1_RW,0x30);        //For optimize RF data transceive
}

void Rfm22TerminalInit()
{
    SPITransmitByte(TRANSMIT_HEADER_3_RW,0xff); 
    SPITransmitByte(TRANSMIT_HEADER_2_RW,0xff); 
    SPITransmitByte(TRANSMIT_HEADER_1_RW,0xff); 
    SPITransmitByte(TRANSMIT_HEADER_0_RW,0xff); 
    //check only header3's bits when recieve packet.
    SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
    SPITransmitByte(HEADER_ENABLE_2_RW,0x00);
    SPITransmitByte(HEADER_ENABLE_1_RW,0x00);
    SPITransmitByte(HEADER_ENABLE_0_RW,0x00);
    
    SPITransmitByte(TRANSMIT_HEADER_3_RW,0xff); 
    SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
    
    //initializing terminal
    SPITransmitByte(HEADER_CONTROL_1_RW,CHECK_BROAD_3 | HEAD3);
    SPITransmitByte(HEADER_CONTROL_2_RW,SYNC3210 | HEAD3);
    SPITransmitByte(CHECK_HEADER_3_RW,0xFE);
}

void Rfm22NodeInit()
{
    //Prepare broadcast headers.
    SPITransmitByte(TRANSMIT_HEADER_3_RW,0xFE);
    SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
    SPITransmitByte(HEADER_ENABLE_2_RW,0x00);
    SPITransmitByte(HEADER_ENABLE_1_RW,0x00);
    SPITransmitByte(HEADER_ENABLE_0_RW,0x00);
    SPITransmitByte(HEADER_CONTROL_1_RW,CHECK_BROAD_3 | CHECK_HEADER_3);
    SPITransmitByte(HEADER_CONTROL_2_RW,SYNC3210 | HEAD3);
    SPITransmitByte(CHECK_HEADER_3_RW,0x01);
}

void RFM_interrupt_RX (void)
{
  SPIReceiveByte(INTERRUPT_STATUS_1_R);                                           //reading and clering interrupt registers
  SPIReceiveByte(INTERRUPT_STATUS_2_R);    	                                //reading and clering interrupt registers
  SPITransmitByte(INTERRUPT_ENABLE_1_RW, IPKVALID /*| ICRCERROR*/); 	                //enable interrupt on receive packet and crc error
  SPITransmitByte(IO_PORT_CONFIGURATION_RW,RX_ANT);		                //SET RFM GPIO to HIGH on RX_ANT
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_1_RW, RECIEVE_MODE | TUNE_MODE);	//transfer to Recieve mode with staing in Ready mode after reception packet
}

void RFM_interrupt_TX (void)
{
  
  SPIReceiveByte(INTERRUPT_STATUS_1_R);                 //reading and clering interrupt registers
  SPIReceiveByte(INTERRUPT_STATUS_2_R);    	        //reading and clering interrupt registers                                        
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x04); 		//enable interrupt on end of transmit packet
}

void SendPacket(uint8* data,uint8 size)
{
  SPITransmitByte(TRANSMIT_PACKET_LENGTH_RW,size);
  uint8 i;                                                              
  for (i=0; i<size; i++)			
  {										
    SPITransmitByte(FIFO_ACCESS_RW, data[i]);		            // transfer data paccket to TX_FIFO
  }
  //SPIBurstTransmit(FIFO_ACCESS_RW,data,size);			//burst filling of buffer
  SPIReceiveByte(INTERRUPT_STATUS_1_R);                  //reading and clering interrupt registers
  SPIReceiveByte(INTERRUPT_STATUS_2_R);    	        //reading and clering interrupt registers     
  SPITransmitByte(IO_PORT_CONFIGURATION_RW,TX_ANT);
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_1_RW,TRANSMIT_MODE | READY_MODE); //transmitting TX_FIFO     
  while(Event_RFM){};						    //waiting for completion of transmission
  //RFM_interrupt_TX();	                           // reset interrupt
  
}
/*void SendPackeT(uint8* data,uint8 start, uint8 end)
{
  SPITransmitByte(TRANSMIT_PACKET_LENGTH_RW,end-start);
  //uint8 i;                                                              
  for (start; start<end; start++)			
  {										
    SPITransmitByte(FIFO_ACCESS_RW, data[start]);		            // transfer data paccket to TX_FIFO
  }
  //SPIBurstTransmit(FIFO_ACCESS_RW,data,size);			//burst filling of buffer
  SPIReceiveByte(INTERRUPT_STATUS_1_R);                  //reading and clering interrupt registers
  SPIReceiveByte(INTERRUPT_STATUS_2_R);    	        //reading and clering interrupt registers     
  SPITransmitByte(IO_PORT_CONFIGURATION_RW,TX_ANT);
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_1_RW,TRANSMIT_MODE | READY_MODE); //transmitting TX_FIFO     
  while(Event_RFM);						    //waiting for completion of transmission
  RFM_interrupt_RX();							//for waiting acknowledgement
}*/
void SuperSendPacket(uint8* dest_address,uint8* data,uint8 size)
{
  //	uint8 temp=0,start=0;
  //	if(size>50)
  //	{
  //		
  //	}
  //	SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  //    SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
  //for direct writing without aTxPacket
  //	SPITransmitByte(FIFO_ACCESS_RW,0x00);
  //	SPITransmitByte(FIFO_ACCESS_RW,0x00);
  //	SPITransmitByte(FIFO_ACCESS_RW,0x00);
  //	SPITransmitByte(FIFO_ACCESS_RW,TERMINAL_ID);     //SA
  //	SPITransmitByte(FIFO_ACCESS_RW,0x00);
  //	SPITransmitByte(FIFO_ACCESS_RW,0x00);
  //	SPITransmitByte(FIFO_ACCESS_RW,0x00);
  //	SPITransmitByte(FIFO_ACCESS_RW,BROAD);     		//DA
  //	SPITransmitByte(FIFO_ACCESS_RW,INITIALIZE);				//Type of package
  //	SPITransmitByte(FIFO_ACCESS_RW,0x00);				//Status of package
  //	SPITransmitByte(FIFO_ACCESS_RW,0x00);				//waylength
  //	for(temp;temp<size;temp++)
  //	{
  //		aTxPacket[start++]=data[temp];
  //	}
  //	SendPackeT(aTxPacket,0,temp-1);
  
  
}
uint8 ReceivePacket (uint8* data)
{
  uint8 length;
  SPITransmitByte(IO_PORT_CONFIGURATION_RW,0x00);
  length=SPIReceiveByte(RECEIVED_PACKET_LENGTH_R);						//Получения кол-ва байт FIFO буфера
  uint8 i;
  for (i=0; i<length; i++)
  {
    data[i]=SPIReceiveByte(FIFO_ACCESS_RW);		                //чтение из FIFO буфера
  }
  RFM_interrupt_RX();						        //сброс прерываний
  while(!Event_RFM);
  return length;								//возвращение кол-ва принятых байт
}
// нужно иметь аналог с таймером и без
void RX_ON (void)
{
  //            static uint8 i1,i2=0;
  RFM_interrupt_RX();
  while(Event_RFM && !TIM3_SR1_UIF);
  
  //Signal strength must be measure after valid preamble detect event
  // SPIReceiveByte(RECEIVED_SIGNAL_STRENGTH_INDICATOR_R);        
  //        i1=SPIReceiveByte(INTERRUPT_STATUS_1_R);                                         //reading and clering interrupt registers
  //        i2=SPIReceiveByte(INTERRUPT_STATUS_2_R);
}
void RX_ON_TIMEOUT (uint16 nTime)
{
  //setting timeout
  //uint16 nTime=6000;                 //Set time out for wait response in miliseconds
  TIM3_PSCR= TIM_PRESCALER_16384;                   //set prescaler to 16384. The estimated frequency is 16MHz
  TIM3_ARRH = (char)(nTime >> 8);    //set limit of count
  TIM3_ARRL = (char)(nTime);
  TIM3_EGR|=MASK_TIM3_EGR_UG;
  TIM3_CR1|=MASK_TIM3_CR1_CEN | MASK_TIM3_CR1_URS | MASK_TIM3_CR1_OPM;    //Counter enable and Set One pulse mode and enable update only when overflow even occur
  TIM3_SR1&=~MASK_TIM3_SR1_UIF;  
  
  RFM_interrupt_RX();
  while(Event_RFM && !TIM3_SR1_UIF); 
  TIM3_CR1&=~MASK_TIM3_CR1_CEN;       //stop counter
}
void TX_ON (void)
{
  RFM_interrupt_TX();
  while(!Event_RFM);
}

void READY_ON (void)
{
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_1_RW, READY_MODE);
}

void WriteToFIFO(uint8* aData,uint8 size)
{
  uint8 i;                                                              
  for (i=0; i<size; i++)			
  {										
    SPITransmitByte(FIFO_ACCESS_RW, aData[i]);		            // transfer data paccket to TX_FIFO
  }
}
void ReadFromFIFO(uint8* aData,uint8 size)
{
  uint8 i;
  for (i=0; i<size; i++)
  {
    aData[i]=SPIReceiveByte(FIFO_ACCESS_RW);		                //чтение из FIFO буфера
  }
}



void RfmSetup868Dev30()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x1b);
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);
  //SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0xc7);			//0x80 to optimize rx sensivity by IF width filter.  0x02 to reduce noise variation on measuring RSSI. 0x04 default. 0x01 for reset Preamble detector after 3 conclusion failure 
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xe8);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x60);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x20);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0xc5);
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x06);
  SPITransmitByte(AFC_LIMITER_RW,0x1c);
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);			//enable CRC CCITT
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			//header 0&1&2&3 Sync 0&1&2&3
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2c);
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x30);
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x73);
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x76);
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0xc0);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  //SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}
void RfmSetup868Dev40Manchester()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x1c);
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);
  //SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0xc7);			//0x80 to optimize rx sensivity by IF width filter.  0x02 to reduce noise variation on measuring RSSI. 0x04 default. 0x01 for reset Preamble detector after 3 conclusion failure  
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xf4);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x20);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x20);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0xc5);
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x0f);
  SPITransmitByte(AFC_LIMITER_RW,0x1e);
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);		//enable CRC CCITT
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			//header 0&1&2&3 Sync 0&1&2&3
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2e);
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x40);
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x73);
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x76);
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0xc0);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}
void RfmSetup433Dev15Manchester()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x2B);
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);
  //SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0xc7);			//0x80 to optimize rx sensivity by IF width filter.  0x02 to reduce noise variation on measuring RSSI. 0x04 default. 0x01 for reset Preamble detector after 3 conclusion failure 
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xf4);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x00);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x41);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x89);
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x48);
  SPITransmitByte(AFC_LIMITER_RW,0x1d);
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			//header 0&1&2&3 Sync 0&1&2&3
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2e);
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}
void RfmSetup433Dev15White()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x2B);
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);
  //SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0xc7);			//0x80 to optimize rx sensivity by IF width filter.  0x02 to reduce noise variation on measuring RSSI. 0x04 default. 0x01 for reset Preamble detector after 3 conclusion failure 
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xf4);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x20);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x41);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x89);
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x13);
  SPITransmitByte(AFC_LIMITER_RW,0x1d);
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			//header 0&1&2&3 Sync 0&1&2&3
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2d);
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
  
  //   SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x10); //this mode of reception of several packages
}
void RfmSetup433Dev50()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x1e);
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);
  //SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0xc7);			//0x80 to optimize rx sensivity by IF width filter.  0x02 to reduce noise variation on measuring RSSI. 0x04 default. 0x01 for reset Preamble detector after 3 conclusion failure 
  
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xe8);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x60);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x20);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0xc5);
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x05);
  SPITransmitByte(AFC_LIMITER_RW,0x20);
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);		//enable CRC CCITT
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			//header 0&1&2&3 Sync 0&1&2&3
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2c);
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x50);
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}
void RfmSetup433Dev30()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x16);
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);
  //SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0xc7);			//0x80 to optimize rx sensivity by IF width filter.  0x02 to reduce noise variation on measuring RSSI. 0x04 default. 0x01 for reset Preamble detector after 3 conclusion failure 
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xe8);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x60);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x20);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0xc5);
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x06);
  SPITransmitByte(AFC_LIMITER_RW,0x1e);
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);		//enable CRC CCITT
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			//header 0&1&2&3 Sync 0&1&2&3
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2c);
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x30);
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}

void RfmSetup433Dev15()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x2b);
  //SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0xc7);			//0x80 to optimize rx sensivity by IF width filter.  0x02 to reduce noise variation on measuring RSSI. 0x04 default. 0x01 for reset Preamble detector after 3 conclusion failure 
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x44);	
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xf4);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x20);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x41);
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x89);
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x13);
  SPITransmitByte(AFC_LIMITER_RW,0x1d);
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0x8c);			//enable CRC CCITT
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			//header 0&1&2&3 Sync 0&1&2&3
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for 0 rssi_ ofset
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2c);
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);
  
  //MY PERSON ADDED
  SPITransmitByte(AGC_OVERRIDE_1_RW,0x30);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}

void RfmSetup868Dev15Manchester()
{
    RfmSetup868Dev40Manchester();
    SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);
    SPITransmitByte(IF_FILTER_BANDWITH_RW,0x1B);
    SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x25);
}

void RfmSetup868Dev15()
{
    RfmSetup868Dev30();
    SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);
    SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x0B);
}

//////////////////////////////////////////////////////////////////////////////////////////
//--------- NEW FOR TESTS  ---------------------------------------------------------//////
//////////////////////////////////////////////////////////////////////////////////////////

/*void RfmSetup433Dev15()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x2b);					//0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x44);			//0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xf4);	//0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x20);				//0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x41);				//0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x89);				//0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);		//0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x13);		//0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1d);							//0x2a
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);					//0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);					//0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);					//0x33

  //----------------------------------------------
  
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for 0 rssi_ ofset
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  //-----------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);				//0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);				//0x6f
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2c);	//0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);	//0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);			//0x72
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);		//0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);	//0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);	//0x77
  //---------------------------------------------------------------------
  
  //MY PERSON ADDED
  SPITransmitByte(AGC_OVERRIDE_1_RW,0x30);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
  
}
*/
void RfmSetup433Dev5()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x2b);					//0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);			//0x1D
  //SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0xc7);
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xf4);	//0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x20);				//0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x41);				//0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x89);				//0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);		//0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x36);		//0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1d);							//0x2a
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);					//0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);					//0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);					//0x33
  //----------------------------------------------
  
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for 0 rssi_ ofset
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  //-----------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);				//0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);				//0x6f
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2c);	//0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);	//0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x08);			//0x72
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);		//0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);	//0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);	//0x77
  //---------------------------------------------------------------------
  
  //MY PERSON ADDED
  SPITransmitByte(AGC_OVERRIDE_1_RW,0x30);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
  
}

void RfmSetup433Dev15_1kRate()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x2b);					//0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);			//0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xe8);	//0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x60);				//0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x20);				//0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0xc5);				//0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);		//0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x06);		//0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1d);							//0x2a
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);					//0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);					//0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);					//0x33
  
  //----------------------------------------------
  
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for 0 rssi_ ofset
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  //-----------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x08);				//0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x31);				//0x6f
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2c);	//0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);	//0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);			//0x72
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);		//0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);	//0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);	//0x77

  //---------------------------------------------------------------------
  
  //MY PERSON ADDED
  SPITransmitByte(AGC_OVERRIDE_1_RW,0x30);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
  
}

void RfmSetup433Dev15_4kRate()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x2b);					//0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);			//0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xfa);	//0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x00);				//0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x83);				//0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x12);				//0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);		//0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x48);		//0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1d);							//0x2a
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);					//0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);					//0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);					//0x33

  //----------------------------------------------
  
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for 0 rssi_ ofset
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  //-----------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x20);				//0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0xc5);				//0x6f
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2c);	//0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);	//0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);			//0x72
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);		//0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);	//0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);	//0x77
  //---------------------------------------------------------------------
  
  //MY PERSON ADDED
  SPITransmitByte(AGC_OVERRIDE_1_RW,0x30);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
  
}

void RfmSetup433Dev15_8kRate()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x2b);					//0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);			//0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0x7d);	//0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x01);				//0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x06);				//0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x25);				//0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x01);		//0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x1a);		//0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1e);							//0x2a
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);					//0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);					//0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);					//0x33
  //----------------------------------------------
  
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for 0 rssi_ ofset
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  //-----------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x41);				//0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x89);				//0x6f
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2c);	//0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);	//0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);			//0x72
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);		//0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);	//0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);	//0x77
  //---------------------------------------------------------------------
  
  //MY PERSON ADDED
  SPITransmitByte(AGC_OVERRIDE_1_RW,0x30);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
  
}

void RfmSetup433Dev15_433MHz()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x27);					//0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);			//0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xf4);	//0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x20);				//0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x41);				//0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x89);				//0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);		//0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x13);		//0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1d);							//0x2a
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);					//0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);					//0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);					//0x33
  //----------------------------------------------
  
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for 0 rssi_ ofset
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  //-----------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);				//0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);				//0x6f
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2c);	//0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);	//0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);			//0x72
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);		//0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x4b);	//0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x00);	//0x77
  //---------------------------------------------------------------------
  
  //MY PERSON ADDED
  SPITransmitByte(AGC_OVERRIDE_1_RW,0x30);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
  
}

void RfmSetup433Dev15_420MHz()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x27);					//0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);			//0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xf4);	//0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x20);				//0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x41);				//0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x89);				//0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);		//0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x13);		//0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1b);							//0x2a
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);					//0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);					//0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);					//0x33
  //----------------------------------------------
  
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for 0 rssi_ ofset
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  //-----------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);				//0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);				//0x6f
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2c);	//0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);	//0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);			//0x72
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x52);		//0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x00);	//0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x00);	//0x77
  //---------------------------------------------------------------------
  
  //MY PERSON ADDED
  SPITransmitByte(AGC_OVERRIDE_1_RW,0x30);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
  
}
void RfmSetup433Dev15_450MHz()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x2B);					//0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);			//0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xf4);	//0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x20);				//0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x41);				//0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x89);				//0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);		//0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x13);		//0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1e);							//0x2a
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);					//0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);					//0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);					//0x33
  //----------------------------------------------
  
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for 0 rssi_ ofset
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  //-----------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);				//0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);				//0x6f
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2c);	//0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);	//0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);			//0x72
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x55);		//0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x00);	//0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x00);	//0x77
  //---------------------------------------------------------------------
  
  //MY PERSON ADDED
  SPITransmitByte(AGC_OVERRIDE_1_RW,0x30);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
  
}

/*void RfmSetup433Dev15Manchester()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x2B);                  //0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);         //0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xf4);   //0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x00);             //0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x41);             //0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x89);             //0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);        //0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x48);        //0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1d);                         //0x2a
                                                                
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);                 //0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);                    //0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			        //0x33
  //------------------------------------------------------------------------------------
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  //-----------------------------------------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);               //0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);               //0x6f
                                                         
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2e);    //0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);    //0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);          //0x72
                                                         
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);         //0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);  //0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);  //0x77
  //--------------------------------------------------------------------------
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}
*/
void RfmSetup433Dev5Manchester()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x2B);                  //0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);         //0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xf4);   //0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x00);             //0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x41);             //0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x89);             //0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);        //0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0xd4);        //0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1d);                         //0x2a
                                                                
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);                 //0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);                    //0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			        //0x33
  //------------------------------------------------------------------------------------
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  //-----------------------------------------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);               //0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);               //0x6f
                                                         
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2e);    //0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);    //0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x08);          //0x72
                                                         
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);         //0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);  //0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);  //0x77
  //--------------------------------------------------------------------------
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}

void RfmSetup433Dev30Manchester()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x16);                  //0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);         //0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xf4);   //0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x20);             //0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x20);             //0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0xc5);             //0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);        //0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x13);        //0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1e);                         //0x2a
                                                                
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);					//0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);					//0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);					//0x33

  //------------------------------------------------------------------------------------
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  //-----------------------------------------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);               //0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);               //0x6f
                                                         
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2e);    //0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);    //0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x30);          //0x72
                                                         
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);         //0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);  //0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);  //0x77
  //--------------------------------------------------------------------------
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}

void RfmSetup433Dev50Manchester()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0xAB);                  //0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);         //0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xee);   //0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x40);             //0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x15);             //0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0xd8);             //0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);        //0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x09);        //0x25
  SPITransmitByte(AFC_LIMITER_RW,0x24);                         //0x2a
                                                                
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);					//0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);					//0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);					//0x33

  //------------------------------------------------------------------------------------
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  //-----------------------------------------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);               //0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);               //0x6f
                                                         
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2e);    //0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);    //0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x50);          //0x72
                                                         
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);         //0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);  //0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);  //0x77
  //-----------------------------------------------------------------------------
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}

void RfmSetup433Dev15Manchester_256kRate()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x8D);                  //0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);         //0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0x17);   //0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x02);             //0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0xbb);             //0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x0d);             //0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x07);        //0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0xff);        //0x25
  SPITransmitByte(AFC_LIMITER_RW,0x50);                         //0x2a
                                                                
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);                 //0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);                    //0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			        //0x33

  //------------------------------------------------------------------------------------
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  //-----------------------------------------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x41);               //0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x89);               //0x6f
                                                         
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x0e);    //0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);    //0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);          //0x72
                                                         
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);         //0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);  //0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);  //0x77
  //--------------------------------------------------------------------------
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}
void RfmSetup433Dev15Manchester_4kRate()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x2B);                  //0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);         //0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0x7d);   //0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x00);             //0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x83);             //0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x12);             //0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x01);        //0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x1a);        //0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1e);                         //0x2a
																
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);                 //0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);                    //0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			        //0x33
  //------------------------------------------------------------------------------------
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  //-----------------------------------------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x20);               //0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0xc5);               //0x6f
                                                         
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2e);    //0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);    //0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);          //0x72
                                                         
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);         //0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);  //0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);  //0x77
  //--------------------------------------------------------------------------
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}

void RfmSetup433Dev15Manchester_8kRate()
{
	
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x2D);                  //0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);         //0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0x3f);   //0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x01);             //0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x06);             //0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x25);             //0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x04);        //0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x58);        //0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1e);                         //0x2a

  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);                 //0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);                    //0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			        //0x33
  //------------------------------------------------------------------------------------

  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  //-----------------------------------------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x41);               //0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x89);               //0x6f
                                                         
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2e);    //0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);    //0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);          //0x72
                                                         
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);         //0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);  //0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);  //0x77
  //--------------------------------------------------------------------------
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}
void RfmSetup433Dev15Manchester_433MHz()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x27);                  //0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);         //0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xfa);   //0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x00);             //0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x41);             //0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x89);             //0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);        //0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x48);        //0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1d);                         //0x2a
                                                                
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);                 //0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);                    //0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			        //0x33
  //------------------------------------------------------------------------------------

  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  //-----------------------------------------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);               //0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);               //0x6f
                                                         
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2e);    //0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);    //0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);          //0x72
                                                         
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);         //0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x4B);  //0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x00);  //0x77
  //--------------------------------------------------------------------------

  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}

void RfmSetup433Dev15Manchester_420MHz()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x27);                  //0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);         //0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xfa);   //0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x00);             //0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x41);             //0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x89);             //0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);        //0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x48);        //0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1d);                         //0x2a
                                                                
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);                 //0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);                    //0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			        //0x33
  //------------------------------------------------------------------------------------

  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  //-----------------------------------------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);               //0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);               //0x6f
                                                         
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2e);    //0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);    //0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);          //0x72
                                                         
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x52);         //0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x00);  //0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x00);  //0x77
  //--------------------------------------------------------------------------

  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}

void RfmSetup433Dev15Manchester_450MHz()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x2B);                  //0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);         //0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xfa);   //0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x00);             //0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x41);             //0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x89);             //0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);        //0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x48);        //0x25
  SPITransmitByte(AFC_LIMITER_RW,0x1e);                         //0x2a
                                                                
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);                 //0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);                    //0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			        //0x33
  //------------------------------------------------------------------------------------

  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  //-----------------------------------------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);               //0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);               //0x6f
                                                         
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2e);    //0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);    //0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);          //0x72
                                                         
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x55);         //0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x00);  //0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x00);  //0x77
  //--------------------------------------------------------------------------

  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}
void RfmSetup433Dev15_64kRate()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x04);					//0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);			//0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0x3f);	//0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x02);				//0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x0c);				//0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x4a);				//0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x07);		//0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0xff);		//0x25
  SPITransmitByte(AFC_LIMITER_RW,0x22);							//0x2a
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);					//0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);					//0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);					//0x33
  //----------------------------------------------
  
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for 0 rssi_ ofset
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  //-----------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);				//0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);				//0x6f
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x0c);	//0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);	//0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);			//0x72
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);		//0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);	//0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);	//0x77
  //---------------------------------------------------------------------
  
  //MY PERSON ADDED
  SPITransmitByte(AGC_OVERRIDE_1_RW,0x30);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}

void RfmSetup433Dev75()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0xae);					//0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);			//0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xdc);	//0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0xa0);				//0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x15);				//0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0xd8);				//0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);		//0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x03);		//0x25
  SPITransmitByte(AFC_LIMITER_RW,0x30);							//0x2a
  
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);					//0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);					//0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);					//0x33
  //----------------------------------------------
  
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for 0 rssi_ ofset
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  
  //-----------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);				//0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);				//0x6f
  
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2c);	//0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);	//0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x78);			//0x72
  
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);		//0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);	//0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);	//0x77
  //---------------------------------------------------------------------
  
  //MY PERSON ADDED
  SPITransmitByte(AGC_OVERRIDE_1_RW,0x30);
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}

void RfmSetup433Dev15Manchester_64kRate()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x95);                  //0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);         //0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0x2f);   //0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0x01);             //0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x5d);             //0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0x86);             //0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x07);        //0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0xff);        //0x25
  SPITransmitByte(AFC_LIMITER_RW,0x3c);                         //0x2a
                                                                
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);                 //0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);                    //0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			        //0x33
  //------------------------------------------------------------------------------------
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  //-----------------------------------------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);               //0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);               //0x6f
                                                         
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x0e);    //0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);    //0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0x18);          //0x72
                                                         
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);         //0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);  //0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);  //0x77
  //--------------------------------------------------------------------------
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}

void RfmSetup433Dev100Manchester()
{
  SPITransmitByte(IF_FILTER_BANDWITH_RW,0x9a);                  //0x1C
  SPITransmitByte(AFC_LOOP_GEARSHIFT_OVERRIDE_RW,0x40);         //0x1D
  SPITransmitByte(CLOCK_RECOVERY_OVERSAMPLING_RATIO_RW,0xdc);   //0x20
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_2_RW,0xa0);             //0x21
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_1_RW,0x0a);             //0x22
  SPITransmitByte(CLOCK_RECOVERY_OFFSET_0_RW,0xec);             //0x23
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_1_RW,0x00);        //0x24
  SPITransmitByte(CLOCK_RECOVERY_TIMING_LOOP_0_RW,0x04);        //0x25
  SPITransmitByte(AFC_LIMITER_RW,0x41);                         //0x2a
                                                                
  SPITransmitByte(DATA_ACCESS_CONTROL_RW,0xac);                 //0x30
  SPITransmitByte(HEADER_CONTROL_1_RW,0x8c);                    //0x32
  SPITransmitByte(HEADER_CONTROL_2_RW,0x46);			        //0x33

  //------------------------------------------------------------------------------------
  SPITransmitByte(PREAMBLE_LENGTH_RW,0x08);
  SPITransmitByte(PREAMBLE_DETECTION_CONTROL_RW,0x28);  //0x#8 for0 rssi_ ofset 
  
  SPITransmitByte(SYNC_WORD_3_RW,'s');
  SPITransmitByte(SYNC_WORD_2_RW,'y');
  SPITransmitByte(SYNC_WORD_1_RW,'n');
  SPITransmitByte(SYNC_WORD_0_RW,'c');
  
  SPITransmitByte(TRANSMIT_HEADER_3_RW,'s');
  SPITransmitByte(TRANSMIT_HEADER_2_RW,'o');
  SPITransmitByte(TRANSMIT_HEADER_1_RW,'n');
  SPITransmitByte(TRANSMIT_HEADER_0_RW,'g');
  
  SPITransmitByte(CHECK_HEADER_3_RW,'s');
  SPITransmitByte(CHECK_HEADER_2_RW,'o');
  SPITransmitByte(CHECK_HEADER_1_RW,'n');
  SPITransmitByte(CHECK_HEADER_0_RW,'g');
  
  SPITransmitByte(HEADER_ENABLE_3_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_2_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_1_RW,0xff);
  SPITransmitByte(HEADER_ENABLE_0_RW,0xff);
  //-----------------------------------------------------------------------------
  SPITransmitByte(TX_DATA_RATE_1_RW,0x10);               //0x6e
  SPITransmitByte(TX_DATA_RATE_0_RW,0x62);               //0x6f
                                                         
  SPITransmitByte(MODULATION_MODE_CONTROL_1_RW,0x2e);    //0x70
  SPITransmitByte(MODULATION_MODE_CONTROL_2_RW,0x23);    //0x71
  SPITransmitByte(FREQUENCY_DEVIATION_RW,0xa0);          //0x72
                                                         
  SPITransmitByte(FREQUENCY_BAND_SELEC_RW,0x53);         //0x75
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_1_RW,0x70);  //0x76
  SPITransmitByte(NOMINAL_CARRIER_FREQUENCY_0_RW,0x80);  //0x77

  //--------------------------------------------------------------------------
  
  //borders of  fifo
  SPITransmitByte(TX_FIFO_CONTROL_1_RW,0x3e);                             // прерывание для TX FIFO верхнее 62 байта
  SPITransmitByte(TX_FIFO_CONTROL_2_RW,0x00);                            // прерывание для TX FIFO нижнее 0 байт
  SPITransmitByte(RX_FIFO_CONTROL_RW,0x08); 
  // Setting RFM_GPIO
  SPITransmitByte(GPIO_0_CONFIGURATION_RW,0x0a);
  SPITransmitByte(GPIO_1_CONFIGURATION_RW,0x0a);
  //Disable all interrupts
  SPITransmitByte(INTERRUPT_ENABLE_1_RW,0x00);
  SPITransmitByte(INTERRUPT_ENABLE_2_RW,0x00);
  
  SPITransmitByte(TX_POWER_RW,0x08|POWER_20dB);							//0x08 to disable lna in TX mode
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x03);    //fifo reset
  SPITransmitByte(OPERATION_FUNCTION_CNTRL_2_RW,0x00); 
}