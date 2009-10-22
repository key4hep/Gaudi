#ifndef _WIN32
#define __cdecl
#define __stdcall

#include "GaudiKernel/Kernel.h"

#ifdef __cplusplus
extern "C"    {
#endif

  //char * __cdecl _i64toa (__int64 val,char *buf,int radix);
  //char * __cdecl _ul64toa (unsigned __int64 val,char *buf,int radix);
GAUDI_API char * __cdecl _itoa (int val,char *buf,int radix);
GAUDI_API char * __cdecl _ltoa (long val,char *buf,int radix);
GAUDI_API char * __cdecl _ultoa (unsigned long val,char *buf,int radix);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#include <cstdlib>
#define _ecvt ecvt

#endif  /* WIN32 */
