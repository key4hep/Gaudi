#include "GaudiKernel/Sleep.h"
#include <chrono>
#include <thread>

namespace Gaudi {

  void Sleep( int sec ) { std::this_thread::sleep_for( std::chrono::seconds( sec ) ); }

  void NanoSleep( long long nsec ) { std::this_thread::sleep_for( std::chrono::nanoseconds( nsec ) ); }
} // namespace Gaudi
