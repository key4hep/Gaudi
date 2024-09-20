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
/*
 * TestToolFailing.cpp
 *
 *  @author: Marco Clemencic
 *  @date: 2008-10-22
 */

// from Gaudi
#include <GaudiKernel/AlgTool.h>

#include "ITestTool.h"

class TestToolFailing : public extends<AlgTool, ITestTool> {

public:
  using extends::extends;

  /// Initialize method
  StatusCode initialize() override;
};

// Declaration of the Tool Factory
DECLARE_COMPONENT( TestToolFailing )

StatusCode TestToolFailing::initialize() {
  debug() << "Initialize" << endmsg;
  if ( !base_class::initialize() ) return StatusCode::FAILURE;

  error() << "Initialization failure" << endmsg;
  return StatusCode::FAILURE;
}
