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
// Include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/MsgStream.h"

#include "TemplatedAlg.h"
#include <vector>

// Static Factory declaration
typedef TemplatedAlg<int, std::vector<std::string>> t1;
typedef TemplatedAlg<double, bool>                  t2;

DECLARE_COMPONENT( t1 )
DECLARE_COMPONENT_WITH_ID( t1, "TAlgIS" )
DECLARE_COMPONENT( t2 )
DECLARE_COMPONENT_WITH_ID( t2, "TAlgDB" )

//------------------------------------------------------------------------------
template <typename T, typename R>
StatusCode TemplatedAlg<T, R>::initialize() {
  //------------------------------------------------------------------------------
  using Gaudi::PluginService::Details::demangle;
  info() << "Initializing TemplatedAlg instance " << name() << " of type " << demangle( typeid( *this ) ) << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
template <typename T, typename R>
StatusCode TemplatedAlg<T, R>::execute() {
  //------------------------------------------------------------------------------
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
template <typename T, typename R>
StatusCode TemplatedAlg<T, R>::finalize() {
  //------------------------------------------------------------------------------
  return StatusCode::SUCCESS;
}
