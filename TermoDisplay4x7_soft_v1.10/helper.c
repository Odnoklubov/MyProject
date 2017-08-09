#include "helper.h"
#include <math.h>
#include <stdlib.h>

void convertHexToStr(UINT32 iValue, UINT8* acByteArray, UINT8 cByteAmt)
{
    UINT8 pos=0;
    UINT32 iValue1 = 0x10;
    for(UINT8 i = 1; i < cByteAmt; i++)
    {
        iValue1 <<= 8;
    }
    while(1)
    {
        switch(iValue/iValue1)
        {
            case 0: acByteArray[pos++]='0'; break;     
            case 1: acByteArray[pos++]='1'; break;
            case 2: acByteArray[pos++]='2'; break;
            case 3: acByteArray[pos++]='3'; break;
            case 4: acByteArray[pos++]='4'; break;
            case 5: acByteArray[pos++]='5'; break;
            case 6: acByteArray[pos++]='6'; break;
            case 7: acByteArray[pos++]='7'; break;
            case 8: acByteArray[pos++]='8'; break;
            case 9: acByteArray[pos++]='9'; break;
            case 0x0A: acByteArray[pos++]='A'; break;
            case 0x0B: acByteArray[pos++]='B'; break;
            case 0x0C: acByteArray[pos++]='C'; break;
            case 0x0D: acByteArray[pos++]='D'; break;
            case 0x0E: acByteArray[pos++]='E'; break;
            case 0x0F: acByteArray[pos++]='F'; break;
        }
        if(iValue1!=1)
        {
            iValue%=iValue1;
            iValue1/=0x10;
        }
        else 
        {
          break;
        }
    }
}
void clearByteArray(UINT8* acByteArray, UINT16 sLength)
{
    for(UINT16 i = 0; i < sLength; i++)
    {
        acByteArray[i] = '\0';
    }
}

void ConvertShortToByteArray(UINT16 sNum, UINT8 *aByteArr)
{
    aByteArr[0] = (sNum >> 8) & 0xFF;
    aByteArr[1] = sNum & 0xFF;
}

void ConvertIntToByteArray(UINT32 sNum, UINT8 *aByteArr)
{
    aByteArr[0] = (UINT8)(sNum >> 24) & 0xFF;
    aByteArr[1] = (UINT8)(sNum >> 16) & 0xFF;
    aByteArr[2] = (UINT8)(sNum >> 8) & 0xFF;
    aByteArr[3] = (UINT8)sNum & 0xFF;
}
UINT16 ConvertByteArrayToShort(UINT8 *aByteArr)
{
    UINT16 iNum = *(UINT16 *)aByteArr;
    return iNum;
}
UINT32 ConvertByteArrayToInt(UINT8 *aByteArr)
{
    UINT32 iNum = *(UINT32 *)aByteArr;
    return iNum;
}

void ConvertIntArrayToByteArray(UINT32 *aIntArray, UINT16 sIntArrLen, UINT8 *aByteArr)
{
     for(UINT16 i = 0; i < sIntArrLen; i++ )
    {
        ConvertIntToByteArray(aIntArray[i], aByteArr + (i<<2));
    }
}

bool IsArrayContainsElement(UINT32 *aIntArray, UINT16 sIntArrLen, UINT32 iElement)
{
    for(UINT16 i = 0; i < sIntArrLen; i++)
    {
        if(aIntArray[i] == iElement)
          return TRUE;
    }
    return FALSE;
}

UINT16 GetEntryNumOfArrayElement(UINT32 *aIntArray, UINT16 sIntArrLen, UINT32 iElement)
{
    for(UINT16 i = 0; i < sIntArrLen; i++)
    {
        if(aIntArray[i] == iElement)
          return i;
    }
    return 0xFFFF;
}

void WriteOneByteArrayToAnother(UINT8 *aOneArray, UINT8 *aTwoArray, UINT16 sArrLen)
{
     for(UINT16 i = 0; i < sArrLen; i++ )
    {
        aOneArray[i] = aTwoArray[i];
    }
}

UINT16 rot16(UINT16 x)
{
    return (x >> 8) | (x << 8);
}

UINT32 rot32(UINT32 x)
{
    return ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | ((x & 0x000000FF) << 24);
}
bool isNumericChar(UINT8 x)
{
    return (x >= '0' && x <= '9')? TRUE: FALSE;
}
UINT32 ConvertStrToInt(UINT8 *str)
{
    if (*str == 0)
       return 0;
 
    UINT32 res = 0;  // Initialize result
    UINT32 sign = 1;  // Initialize sign as positive
    UINT32 i = 0;  // Initialize index of first digit
 
    // If number is negative, then update sign
    if (str[0] == '-')
    {
        sign = -1;
        i++;  // Also update index of first digit
    }
 
    // Iterate through all digits of input string and update result
    for (; str[i] != '\0'; ++i)
    {
        if (isNumericChar(str[i]) == FALSE)
            return 0; // You may add some lines to write error message
                      // to error stream
        res = res*10 + str[i] - '0';
    }
 
    // Return result with sign
    return sign*res;
}
bool IsArrCmp(UINT8 *arr1, UINT8 *arr2, UINT16 sLength)
{
    for(UINT8 i = 0; i < sLength; i++)
    {
        if(arr1[i] != arr2[i])
          return FALSE;
    }
    return TRUE;
}

UINT8 GetDigitNum(UINT16 sNum)
{//  Gets the digit amount of sNum
  return (UINT8)floor (log10 (abs (sNum))) + 1;
}

bool strCompareLen(uint8 str1[],uint8 str2[], uint8 cLen)
{
  
  for(uint8 i = 0; i < cLen; i++)
  {
    if(str1[i] != str2[i])
      return FALSE;
  }
  return TRUE;
}