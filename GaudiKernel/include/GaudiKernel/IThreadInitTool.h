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
#ifndef GAUDIKERNEL_ITHREADINITTOOL_H
#define GAUDIKERNEL_ITHREADINITTOOL_H 1

#include "GaudiKernel/IAlgTool.h"

//-----------------------------------------------------------------------------

/** @class IThreadInitTool GaudiKernel/IThreadInitTool.h
 *
 *  @brief Abstract interface for AlgTools to do thread local initialization.
 *
 *  Tools that implement this interface can be plugged into a ThreadPoolSvc
 *  to be called during the initialization of each worker thread to do
 *  thread-local setup. Correspondingly, the tool will be invoked again at
 *  the end of processing in each thread to do thread-local cleanup or final
 *  work.
 *
 *  @author Charles Leggett
 *  @date   2015-09-01
 */

//-----------------------------------------------------------------------------

class IThreadInitTool : virtual public IAlgTool {

public:
  /// Declare the interface to the framework.
  DeclareInterfaceID( IThreadInitTool, 1, 0 );

  /// Perform worker thread initialization. Called concurrently on each thread.
  virtual void initThread() = 0;

  /// Perform worker thread finalization. Called concurrently on each thread.
  virtual void terminateThread() = 0;

  /** @brief Number of threads initialized.
   *
   *  Used for some error checking. The count must be atomically incremented
   *  during initialization and atomically decremented during finalization.
   */
  virtual unsigned int nInit() const = 0;
};

#endif
