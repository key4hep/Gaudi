/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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

class GAUDI_API IThreadPoolSvc : virtual public IInterface {

public:
  /// InterfaceID
  DeclareInterfaceID( IThreadPoolSvc, 1, 0 );

  /// Initializes the thread pool
  virtual StatusCode initPool( const int& poolSize, const int& maxParallelismExtra ) = 0;

  /// Finalize the thread pool
  virtual StatusCode terminatePool() = 0;

  /// Size of the initialized thread pool
  virtual int poolSize() const = 0;

  /// Do thread local initialization of current thread.
  /// Should only be used if it's determined that it has not
  /// already been done.
  virtual void initThisThread() = 0;
};

#endif
