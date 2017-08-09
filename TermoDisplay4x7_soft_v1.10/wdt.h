#ifndef __WDT_H_
 #define __WDT_H_

#define WDT_KEY_ENABLE                  0xCC
#define WDT_KEY_REFRESH                 0xAA
#define WDT_KEY_ACCESS                  0x55

/*typedef enum
{
  WDT_PRESCALER_4 = 0,
  WDT_PRESCALER_8 = 1,
  WDT_PRESCALER_16 = 2,
  WDT_PRESCALER_32 = 3,
  WDT_PRESCALER_64 = 4,
  WDT_PRESCALER_128 = 5,
  WDT_PRESCALER_256 = 6,
};*/

// possible timeouts of WDT
typedef enum
{
  WDT_TIMEOUT_127ms = 0,
  WDT_TIMEOUT_255ms,
  WDT_TIMEOUT_510ms,
  WDT_TIMEOUT_1s02,
} WDT_TIMEOUT;

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Initialize WDT
//!
//! \param enTimeout - WDT Timeout
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
void WdtInit(WDT_TIMEOUT enTimeout);

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Resets WDT
//!
//! \param none
//!
//! \return void
//!
////////////////////////////////////////////////////////////////////////////////
//void WdtReset();
#define WdtReset()              {IWDG_KR = WDT_KEY_REFRESH;} // write special key = reset WDT

#endif