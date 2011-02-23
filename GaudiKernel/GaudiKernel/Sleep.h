#ifndef GAUDIKERNEL_SLEEP_H_
#define GAUDIKERNEL_SLEEP_H_

#include "GaudiKernel/Kernel.h"

namespace Gaudi {

  /// Simple sleep function.
  /// @author Marco Clemencic
  GAUDI_API void Sleep(int sec);

  /// Small variation on the sleep function for nanoseconds sleep.
  /// @author Marco Clemencic
  GAUDI_API void NanoSleep(long long nsec);

}

#endif /*GAUDIKERNEL_SLEEP_H_*/
