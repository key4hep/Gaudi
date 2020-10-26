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
///////////////////////// -*- C++ -*- /////////////////////////////
// IIoComponent.h
// Header file for class IIoComponent
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef GAUDIKERNEL_IIOCOMPONENT_H
#define GAUDIKERNEL_IIOCOMPONENT_H 1

/** @class IIoComponent
 */

// GaudiKernel includes
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/StatusCode.h"

class GAUDI_API IIoComponent : virtual public INamedInterface {

public:
  DeclareInterfaceID( IIoComponent, 1, 0 );

  ///////////////////////////////////////////////////////////////////
  // Non-const methods:
  ///////////////////////////////////////////////////////////////////

  /** @brief callback method to reinitialize the internal state of
   *         the component for I/O purposes (e.g. upon @c fork(2))
   */
  virtual StatusCode io_reinit() = 0;

  virtual StatusCode io_finalize() { return StatusCode::SUCCESS; }
};

#endif //> !GAUDIKERNEL_IIOCOMPONENT_H
