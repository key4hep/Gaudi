#ifndef GAUDIKERNEL_CONCURRENCYFLAGS_H
#define GAUDIKERNEL_CONCURRENCYFLAGS_H 1

#include <cstddef>

#include "GaudiKernel/Kernel.h"

class ThreadPoolSvc;
class ForwardSchedulerSvc;

/** @class ConcurrencyFlags ConcurrencyFlags.h GaudiKernel/ConcurrencyFlags.h
 *
 * Provides information about the level of concurrency of the currently
 * executing job
 *
 * will enumerate the number of Worker Threads (for multi-threading), 
 * Processes (for multi-processing), and total number of concurrent events.
 *
 */

namespace Gaudi {

  namespace Concurrency {

    class ConcurrencyFlags {

      friend class ::ThreadPoolSvc;
      friend class ::ForwardSchedulerSvc;

    public:
      
      /** number of Worker Threads (for MT)
       */
      static GAUDI_API std::size_t numThreads() {return n_threads; }

      /** number of Concurrent Events (for MT)
       */
      static GAUDI_API std::size_t numConcurrentEvents() { return n_concEvts; }

      /** number of forked child processes (for MP)
       */
      static GAUDI_API std::size_t numProcs() { return n_procs; }

      /** serial operation, or some form of concurrency
       */
      static GAUDI_API bool concurrent() { 
        return ( n_threads || n_concEvts || n_procs );
      }

    private:

      static GAUDI_API void setNumThreads(const std::size_t& nT) {n_threads=nT;}
      static GAUDI_API void setNumConcEvents(const std::size_t& nE) {n_concEvts=nE;}
      static GAUDI_API void setNumProcs(const std::size_t& nP) {n_procs=nP;}

    private:
      static std::size_t n_threads;     // worker threads for MT
      static std::size_t n_concEvts;    // concurrent events for MT
      static std::size_t n_procs;       // child processes for MP
    };

  }
}

#endif
