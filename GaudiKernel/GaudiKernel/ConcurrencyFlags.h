#ifndef GAUDIKERNEL_CONCURRENCYFLAGS_H
#define GAUDIKERNEL_CONCURRENCYFLAGS_H 1

#include <cstddef>

#include "GaudiKernel/Kernel.h"

class ThreadPoolSvc;
class ForwardSchedulerSvc;

namespace Gaudi {

  namespace Concurrency {

    class ConcurrencyFlags {

      friend class ::ThreadPoolSvc;
      friend class ::ForwardSchedulerSvc;

    public:
      
      static GAUDI_API std::size_t numThreads() {return n_threads; }
      static GAUDI_API std::size_t numConcurrentEvents() { return n_concEvts; }
      static GAUDI_API std::size_t numProcs() { return n_procs; }

    private:

      static GAUDI_API void setNumThreads(const std::size_t& nT) {n_threads=nT;}
      static GAUDI_API void setNumConcEvents(const std::size_t& nE) {n_concEvts=nE;}
      static GAUDI_API void setNumProcs(const std::size_t& nP) {n_procs=nP;}

    private:
      static std::size_t n_threads;
      static std::size_t n_concEvts;
      static std::size_t n_procs;
    };

  }
}

#endif
