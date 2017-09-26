#ifndef GAUDIKERNEL_ITHREADPOOLSVC_H
#define GAUDIKERNEL_ITHREADPOOLSVC_H

#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/IThreadInitTool.h"
#include <vector>

//-----------------------------------------------------------------------------

/** @class IThreadPoolSvc GaudiKernel/IThreadPoolSvc.h
 *
 *  @brief Abstract interface for a service that manages a thread pool.
 *
 *  Also provides a method to retrieve a list of thread initialization tools.
 *  initialization tools.
 *
 *  @author Charles Leggett
 *  @date   2015-09-01
 */

//-----------------------------------------------------------------------------

class GAUDI_API IThreadPoolSvc : virtual public IInterface
{

public:
  /// InterfaceID
  DeclareInterfaceID( IThreadPoolSvc, 1, 0 );

  /// Initializes the thread pool
  virtual StatusCode initPool( const int& poolSize ) = 0;

  /// Finalize the thread pool
  virtual StatusCode terminatePool() = 0;

  /// Size of the initialized thread pool
  virtual int poolSize() const = 0;
};

#endif
