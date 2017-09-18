#include "GaudiKernel/ConcurrencyFlags.h"

namespace Gaudi {
  namespace Concurrency {

    std::size_t ConcurrencyFlags::n_threads  { 0 };
    std::size_t ConcurrencyFlags::n_concEvts { 0 };
    std::size_t ConcurrencyFlags::n_procs    { 0 };

  }
}
