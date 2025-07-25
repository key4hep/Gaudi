/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "ReadHandleAlg.h"
#include <GaudiKernel/ThreadLocalContext.h>

DECLARE_COMPONENT( Gaudi::TestSuite::ReadHandleAlg )

StatusCode Gaudi::TestSuite::ReadHandleAlg::execute() {

  Collision* c = m_inputHandle.get();

  const int evtNum = Gaudi::Hive::currentContext().evt();

  info() << "Event " << evtNum << " Collision number " << c->collision() << endmsg;

  return StatusCode::SUCCESS;
}
