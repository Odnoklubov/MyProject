#ifndef __RTC_H_
 #define __RTC_H_

#include <stdint.h>
#include <stdio.h>
#include "iostm8s105c6.h"
#include "types.h"

#define RTC_RATE_HZ       500 // "������������" �����

#define ms(x)             ((uint32)x*RTC_RATE_HZ/1000)          // ������ ������������� �� � ����
#define tick(x)           ((uint32)x * 1000 / RTC_RATE_HZ)     // ������ ������������� ���� � ��

void vRTCInit();          // ������������� RTC
uint32_t nGetTickCount(); // ���������� ���������� �����, ��������� ����� ������������� RTC

#endif