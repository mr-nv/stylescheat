#include "windows.h"

typedef unsigned long CRC32;

extern CRC32 CRC32_Get(void* pAddr, DWORD dwLength);
extern CRC32 CRC32_Byte(CRC32 crc, BYTE byte);