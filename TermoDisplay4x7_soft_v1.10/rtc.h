///////////////////////////////////////////////////////////////////////////////
//!
//! \file clk.h
//!         TODO: improve descriptions. AT
//! \brief Module for Using timer and Generate real time clock
//!
//! \author Aliaksandr Bandarenko "xxx@mcsys.by"
//!
////////////////////////////////////////////////////////////////////////////////
#ifndef __RTC_H_
#define __RTC_H_

#include "types.h"
#include "iostm8s105c6.h"
#include "Display4x7.h"
#include "rfm22.h"

#define RTC_COUNT_FOR_TIMER     125     // The number of ticks for appearing of timer interrupt. (For time resolution 10 milliseconds.)

#define RTC_RATE_HZ             1000     // Rate of timer overflow (Hz).  (Fosc = 16000000 Hz. Timer pre-scaler = 128, Timer count = 250. RTC_RATE_HZ = Fosc/Timer pre-scaler/Timer count) 
                                        // ticks per 1 second. Note: this should not be changed without changes in vRTCInit!
                                        
#define RTC_SLOW_RATE_HZ        (RTC_RATE_HZ/10) // Rate for slow RTC  (RTC_SLOW_RATE_HZ = 500/10=50 (Hz) = 1/10 sec)                                

// max possible difference between max number of timer ticks and tick number range. 1h for different . (Rate of timer overflow(Hz)* 3600 (seconds) = 1800000)
// if number of RTC ticks more then RTC_TIMER_TICK_MAX_DIF, add to current value RTC_TIMER_TICK_MAX_DIF and start calculation RTC ticks form the beginning.
#define RTC_TIMER_TICK_MAX_DIF  ((UINT32)RTC_RATE_HZ*3600)   
#define UINT32_MAX              0xffffffff
#define UPTIME_MAX              (UINT32_MAX / RTC_SLOW_RATE_HZ)
#define UPTIME_THRESHOLD        (UPTIME_MAX / 100) *91                         //  ~90 days of Uptime


// TODO: remove wrong naming like ms AT
#define ms(x)             ((UINT32)x*RTC_RATE_HZ/1000)    // calc RTC ticks to milliseconds
#define sec(x)            ((UINT32)x*RTC_RATE_HZ)         // calc RTC ticks to seconds 

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Initialize RTC
//!
//! \param none
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void RTCInit();

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Return number of RTC ticks since start
//!
//! \param none
//!
//! \return UINT32 - number of ticks
//!
////////////////////////////////////////////////////////////////////////////////
UINT32 GetTickCount();

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Return number of slow RTC ticks since start
//!
//! \param none
//!
//! \return UINT32 - number slow RTC ticks (1/10 seconds)
//!
////////////////////////////////////////////////////////////////////////////////
UINT32 GetUptime();

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Check If nTick are in the past - return true. Otherwise - false.
//!
//! \param UINT32 nTick - tick marker
//!
//! \return bool    TRUE - nTick is in the past
//!                 FALSE - nTick >= current tick
//!
////////////////////////////////////////////////////////////////////////////////
bool  IsTickPassed(UINT32 nTick);

#endif