
#ifndef _TYPES_H_
#define _TYPES_H_

////////////////////////////////////////////////////////////////////////////////
// Macro definitions
////////////////////////////////////////////////////////////////////////////////

#define __REG8(NAME, ADDRESS)              \
                  __near __no_init volatile unsigned char NAME @ ADDRESS;
                  
#define ENABLE_INTERRUPTS  asm("rim\n"); // reset interrupt mask - enable interrupts with max priority
                                         // load 1:0 in l1:l0 of CCR            
#define DISABLE_INTERRUPTS asm("sim\n"); // set interrupt mask - disable hardware interrupt
                                         // load 1:0 in l1:l0 of CCR 

// initialization variables
#define INIT_ZERO 	0x00    // default initialization by zero
#define INIT_FF 	0xFF    // default initialization by all one in binary system


// masks
#define MASK_0_BIT				0x01
#define MASK_1_BIT				0x02
#define MASK_2_BIT				0x04
#define MASK_3_BIT				0x08
#define MASK_4_BIT				0x10
#define MASK_5_BIT				0x20
#define MASK_6_BIT				0x40
#define MASK_7_BIT				0x80
#define MASK_15_BIT             0x8000

#define MASK_FIRST_TETRADE 		0x0F
#define MASK_SECOND_TETRADE 	0xF0
#define MASK_THIRD_TETRADE 		0xF00
#define MASK_FORTH_TETRADE 		0xF000


// bits
#define SHIFT_TO_ONE_BIT    1
#define SHIFT_TO_HALF_BYTE 	4
#define SHIFT_TO_BYTE 		8

#define BITS_IN_BYTE		8
#define BITS_IN_WORD        16



#define CLEAR_TO_ZERO 	0x00    // set zero parameter

#define TIM_PRESCALER_1       0x00
#define TIM_PRESCALER_2       0x01
#define TIM_PRESCALER_4       0x02
#define TIM_PRESCALER_8       0x03
#define TIM_PRESCALER_16      0x04
#define TIM_PRESCALER_32      0x05
#define TIM_PRESCALER_64      0x06
#define TIM_PRESCALER_128     0x07
#define TIM_PRESCALER_256     0x08
#define TIM_PRESCALER_512     0x09
#define TIM_PRESCALER_1024    0x0A
#define TIM_PRESCALER_2048    0x0B
#define TIM_PRESCALER_4096    0x0C
#define TIM_PRESCALER_8192    0x0D
#define TIM_PRESCALER_16384   0x0E
#define TIM_PRESCALER_32768   0x0F


////////////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////////////
// Signed integer types  
typedef signed char     int8;
typedef signed short    int16;
typedef signed long     int32;

// Unsigned integer types  
typedef unsigned char     uint8;
typedef unsigned short    uint16;
typedef unsigned long     uint32;

typedef enum {FALSE = 0, TRUE = !FALSE} bool;

typedef signed char     INT8;
typedef signed short    INT16;
typedef signed long     INT32;

// Unsigned integer types  
typedef unsigned char     UINT8;
typedef unsigned short    UINT16;
typedef unsigned long     UINT32;

//#define TRUE 0x01
//#define FALSE 0x00
#endif
