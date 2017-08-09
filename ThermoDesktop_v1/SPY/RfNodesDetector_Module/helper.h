#include "types.h"
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief compare two char arrays
//!
//! \param aStr1 - pointer data array of chars
//! \param aStr2 - pointer data array of chars
//!
//! \return TRUE        if arrays are equals
//!         FALSE       if arrays are not equals
//!
////////////////////////////////////////////////////////////////////////////////
BOOL strCompare(UINT8 aStr1[],UINT8 aStr2[]);
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief convert UINT32 value to byte array UINT8
//!
//! \param iValue - integer value
//! \param acByteArray - pointer data array of bytes
//!
////////////////////////////////////////////////////////////////////////////////
void myitoa(UINT32 iValue, UINT8* acByteArray);
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief set all values of byte array UINT8 to '\0'
//!
//! \param acByteArray - pointer data array of bytes
//! \param sLength - length of array of bytes
//!
////////////////////////////////////////////////////////////////////////////////
void clearByteArray(UINT8* acByteArray, UINT16 sLength);
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief convert UINT16 to byte array UINT8
//!
//! \param aByteArr - pointer data array of bytes
//! \param sNum - UINT16 number to convert
//!
////////////////////////////////////////////////////////////////////////////////
void ConvertShortToByteArray(UINT16 sNum, UINT8 *aByteArr);
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief convert UINT32 to byte array UINT8
//!
//! \param aByteArr - pointer data array of bytes
//! \param sNum - UINT32 number to convert
//!
////////////////////////////////////////////////////////////////////////////////
void ConvertIntToByteArray(UINT32 sNum, UINT8 *aByteArr);
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief convert byte array UINT8 to UINT32  
//!
//! \param aByteArr - pointer data array of bytes
//!
////////////////////////////////////////////////////////////////////////////////
UINT32 ConvertByteArrayToInt(UINT8 *aByteArr);
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief convert byte array UINT8 to UINT16  
//!
//! \param aByteArr - pointer data array of bytes
//!
////////////////////////////////////////////////////////////////////////////////
UINT16 ConvertByteArrayToShort(UINT8 *aByteArr);
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief convert int array UINT32 to byie attay UINT8  
//!
//! \param aByteArr - pointer data array of bytes
//!
////////////////////////////////////////////////////////////////////////////////
void ConvertIntArrayToByteArray(UINT32 *aIntArray, UINT16 sIntArrLen, UINT8 *aByteArr);

BOOL IsArrayContainsElement(UINT32 *aIntArray, UINT16 sIntArrLen, UINT32 iElement);

UINT16 GetEntryNumOfArrayElement(UINT32 *aIntArray, UINT16 sIntArrLen, UINT32 iElement);

void WriteOneByteArrayToAnother(UINT8 *aOneArray, UINT8 *aTwoArray, UINT16 sArrLen);

UINT16 rot16(UINT16 x);
UINT32 rot32(UINT32 x);

BOOL isNumericChar(UINT8 x);
UINT32 ConvertStrToInt(UINT8 *str);
void convertHexToStr(UINT32 iValue, UINT8* acByteArray, UINT8 cByteAmt);
BOOL IsArrCmp(UINT8 *arr1, UINT8 *arr2, UINT16 sLength);