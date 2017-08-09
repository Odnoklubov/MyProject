///////////////////////////////////////////////////////////////////////////////
//!
//! \file clk.h
//!
//! \brief Module for clock initialization, base level
//!
//! \author Dmitriy Vorobyov "dima@mcsys.by"
//!
////////////////////////////////////////////////////////////////////////////////
#ifndef CLK_BASE_LEVEL
#define CLK_BASE_LEVEL

#include "types.h"
#include "delay.h"

////////////////////////////////////////////////////////////////////////////////
// Macro definitions
////////////////////////////////////////////////////////////////////////////////

//CPU Frequency
#define F_CPU_Hz                        16000000
#define F_CPU_MHz                       16

////////////////////////////////////////////////////////////////////////////////
// The addresses of registers
////////////////////////////////////////////////////////////////////////////////

__REG8(CLK_ICKR,     0x50C0);
__REG8(CLK_ECKR,     0x50C1);
__REG8(CLK_CMSR,     0x50C3);
__REG8(CLK_SWR,      0x50C4);
__REG8(CLK_SWCR,     0x50C5);
__REG8(CLK_CKDIVR,   0x50C6);
__REG8(CLK_PCKENR1,  0x50C7);
__REG8(CLK_CSSR,     0x50C8);
__REG8(CLK_CCOR,     0x50C9);
__REG8(CLK_PCKENR2,  0x50CA);
__REG8(CLK_CANCCR,   0x50CB);
__REG8(CLK_HSITRIMR, 0x50CC);
__REG8(CLK_SWIMCCR,  0x50CD);

////////////////////////////////////////////////////////////////////////////////
// The masks of registers
////////////////////////////////////////////////////////////////////////////////
// CLK_ICKR (Internal clock control register). Reset status 0x01.
#define MASK_CLK_ICKR_REGAN             0x20 // 0 - MVR regulator ON in active halt mode
                                             // 1 - MVR regulator OFF in active halt mode
#define MASK_CLK_ICKR_LSIRDY            0x10 // 0 - LSI clock not ready
                                             // 1 - LSI clock ready
#define MASK_CLK_ICKR_LSIEN             0x08 // 0 - Low-speed internal RC off
                                             // 1 - Low-speed internal RC on
#define MASK_CLK_ICKR_FHWU              0x04 // 0 - Fast wakeup from halt/active halt modes disabled
                                             // 1 - Fast wakeup from halt/active halt modes enabled
#define MASK_CLK_ICKR_HSIRDY            0x02 // 0 - HSI clock not ready
                                             // 1 - HSI clock ready
#define MASK_CLK_ICKR_HSIEN             0x01 // 0 - High-speed internal RC off
                                             // 1 - High-speed internal RC on

// CLK_ECKR (External clock control register). Reset status 0x00.
#define MASK_CLK_ECKR_HSERDY            0x02 // 0 - HSE clock is not ready
                                             // 1 - HSE clock is ready
#define MASK_CLK_ECKR_HSEEN             0x01 // 0 - HSE clock off
                                             // 0 - HSE clock on

// CLK_CMSR (Clock master status register). Reset status 0xE1.
#define MASK_CLK_CMSR_HSI               0xE1 // HSI selected as master clock source
#define MASK_CLK_CMSR_lSI               0xD2 // LSI selected as master clock source
#define MASK_CLK_CMSR_HSE               0xB4 // HSE selected as master clock source

// CLK_SWR (Clock master switch register). Reset status 0xE1.
#define MASK_CLK_SWR_HSI                0xE1 // HSI selected as master clock source
#define MASK_CLK_SWR_LSI                0xD2 // LSI selected as master clock source
#define MASK_CLK_SWR_HSE                0xB4 // HSE selected as master clock source

// CLK_SWCR (Clock switch control register). Reset status 0xXX.
#define MASK_CLK_SWCR_SWIF              0x08 // In manual switching mode (SWEN = 0)
                                             // 0 - Target clock source not ready
                                             // 1 - Target clock source ready
                                             // In automatic switching mode (SWEN = 1)
                                             // 0 - No clock switch event occurred
                                             // 1 - Clock switch event occurred
#define MASK_CLK_SWCR_SWIEN             0x04 // 0 - Clock switch interrupt disabled
                                             // 1 - Clock switch interrupt enabled
#define MASK_CLK_SWCR_SWEN              0x02 // 0 - Disable clock switch execution
                                             // 1 - Enable clock switch execution
#define MASK_CLK_SWCR_SWBSY             0x01 // This bit is set and cleared by hardware. 
                                             // It can be cleared by software to reset the clock switch process.
                                             // 0 - No clock switch ongoing
                                             // 1 - Clock switch ongoing

// CLK_CKDIVR (Clock divider register). Reset status 0x18.
#define MASK_CLK_CKDIVR_HSIDIV_BIT1     0x10
#define MASK_CLK_CKDIVR_HSIDIV_BIT0     0x08
#define MASK_CLK_CKDIVR_CPUDIV_BIT2     0x04
#define MASK_CLK_CKDIVR_CPUDIV_BIT1     0x02
#define MASK_CLK_CKDIVR_CPUDIV_BIT0     0x01

// CLK_PCKENR1 (Peripheral clock gating register 1). Reset status 0xFF.
// 0 - f(master) to peripheral disabled
// 1 - f(master) to peripheral enabled
#define MASK_CLK_PCKENR1_TIM1           0x80
#define MASK_CLK_PCKENR1_TIM3           0x40
#define MASK_CLK_PCKENR1_TIM2           0x20
#define MASK_CLK_PCKENR1_TIM4           0x10
#define MASK_CLK_PCKENR1_UART2          0x08
#define MASK_CLK_PCKENR1_SPI            0x02
#define MASK_CLK_PCKENR1_I2C            0x01

// CLK_CSSR (Clock security system register). Reset status 0x00.
#define MASK_CLK_CSSR_CSSD              0x08 // This bit is set by hardware and cleared by software writing 0
                                             // 0 - CSS is off or no HSE crystal clock disturbance detected
                                             // 1 - HSE crystal clock disturbance detected
#define MASK_CLK_CSSR_CSSDIE            0x04 // This bit is set and cleared by software
                                             // 0 - Clock security system interrupt disabled
                                             // 1 - Clock security system interrupt enabled
#define MASK_CLK_CSSR_AUX               0x02 // This bit is set and cleared by hardware
                                             // 0 - Auxiliary oscillator is off
                                             // 1 - Auxiliary oscillator (HSI/8) is on and selected as current clock master source
#define MASK_CLK_CSSR_CSSEN             0x01 // This bit can be read many times and be written once-only by software
                                             // 0 - Clock security system off
                                             // 1 - Clock security system on

// CLK_CCOR (Configurable clock output register). Reset status 0x00.
#define MASK_CLK_CCOR_CCOBSY            0x40
#define MASK_CLK_CCOR_CCORDY            0x20
#define MASK_CLK_CCOR_CCOSEL_BIT3       0x10
#define MASK_CLK_CCOR_CCOSEL_BIT2       0x08
#define MASK_CLK_CCOR_CCOSEL_BIT1       0x04
#define MASK_CLK_CCOR_CCOSEL_BIT0       0x02
#define MASK_CLK_CCOR_CCOEN             0x01

// CLK_PCKENR2 (Peripheral clock gating register 2). Reset status 0xFF.
// 0 - f(master) to peripheral disabled
// 1 - f(master) to peripheral enabled
#define MASK_CLK_PCKENR2_ADC            0x08
#define MASK_CLK_PCKENR2_AWU            0x04

// CLK_CANCCR (CAN clock control register). Reset status 0x00.

// CLK_HSITRIMR (HSI clock calibration trimming register). Reset status 0x00.
// These bits are written by software to fine tune the HSI calibration.
#define MASK_CLK_HSITRIMR_BIT3          0x08
#define MASK_CLK_HSITRIMR_BIT2          0x04
#define MASK_CLK_HSITRIMR_BIT1          0x02
#define MASK_CLK_HSITRIMR_BIT0          0x01

// CLK_SWIMCCR (SWIM clock control register). Reset status 0bXXXXXXX0.
#define MASK_CLK_SWIMCCR_SWIMCLK        0x01 // This bit is set and cleared by software.
                                             // 0 - SWIM clock divided by 2
                                             // 1 - SWIM clock not divided by 2

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief The initialization of CPU clock
//!
////////////////////////////////////////////////////////////////////////////////
void ClkInit(void);

#endif //CLK_BASE_LEVEL