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
#ifndef GAUDIKERNEL_IACCELERATOR_H
#define GAUDIKERNEL_IACCELERATOR_H

#include <functional>
#include <vector>

// Framework include files
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/ITask.h"

/**@class IAccelerator IAccelerator.h GaudiKernel/IAccelerator.h
 *
 *  General interface for an accelerator-based algorithm scheduler.
 *
 *  @author  Illya Shapoval
 *  @version 1.0
 */
class GAUDI_API IAccelerator : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IAccelerator, 1, 0 );

  virtual StatusCode push( ITask& task ) = 0;
};
#endif
