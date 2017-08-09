#include "wdt.h"

#include <iostm8s105c6.h>

void WdtInit(WDT_TIMEOUT enTimeout)
{
  // need to disable interrupt to access WDT correctly
  //__disable_interrupts();
  
  // enable WDT
  IWDG_KR = WDT_KEY_ENABLE;
  IWDG_KR = WDT_KEY_REFRESH;
  
  // Write new reload value - no need to do this as reset value is 0xFF
//  IWDG_KR = WDT_KEY_ACCESS;
//  IWDG_RLR = 0xFF;

  // set prescaler according desired timeout
  switch (enTimeout)
  {
    case WDT_TIMEOUT_127ms:
      IWDG_KR = WDT_KEY_ACCESS;
      IWDG_PR = 3;
      break;
    case WDT_TIMEOUT_255ms:
      IWDG_KR = WDT_KEY_ACCESS;
      IWDG_PR = 4;
      break;
    case WDT_TIMEOUT_510ms:
      IWDG_KR = WDT_KEY_ACCESS;
      IWDG_PR = 5;
      break;
    case WDT_TIMEOUT_1s02:
      IWDG_KR = WDT_KEY_ACCESS;
      IWDG_PR = 6;
      break;
  }

  //__enable_interrupts();
}

/*void WdtReset()
{
  IWDG_KR = WDT_KEY_REFRESH; // write special key = reset WDT
}*/