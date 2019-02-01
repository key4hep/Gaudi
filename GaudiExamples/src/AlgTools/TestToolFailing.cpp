/*
 * TestToolFailing.cpp
 *
 *  @author: Marco Clemencic
 *  @date: 2008-10-22
 */

// from Gaudi
#include "GaudiAlg/GaudiTool.h"

#include "ITestTool.h"

class TestToolFailing : public extends<GaudiTool, ITestTool> {

public:
  using extends::extends;

  /// Initialize method
  StatusCode initialize() override;
};

// Declaration of the Tool Factory
DECLARE_COMPONENT( TestToolFailing )

StatusCode TestToolFailing::initialize() {
  debug() << "Initialize" << endmsg;
  if ( !GaudiTool::initialize() ) return StatusCode::FAILURE;

  return Error( "Initialization failure" );
}
