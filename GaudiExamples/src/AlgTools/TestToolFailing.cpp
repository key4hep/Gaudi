/*
 * TestToolFailing.cpp
 *
 *  @author: Marco Clemencic
 *  @date: 2008-10-22
 */

// from Gaudi
#include "GaudiAlg/GaudiTool.h"

#include "ITestTool.h"


class TestToolFailing: public extends1<GaudiTool, ITestTool> {

public:
  TestToolFailing(const std::string& type,
                  const std::string& name,
                  const IInterface* parent);

  /// Initialize method
  StatusCode initialize();

  virtual ~TestToolFailing( ) {} ///< Destructor

};

#include "GaudiKernel/ToolFactory.h"
// Declaration of the Tool Factory
DECLARE_COMPONENT(TestToolFailing)

TestToolFailing::TestToolFailing(const std::string& type,
                                 const std::string& name,
                                 const IInterface* parent):
  base_class(type, name, parent) {
}

StatusCode TestToolFailing::initialize(){
  debug() << "Initialize" << endmsg;
  if ( !GaudiTool::initialize() ) return StatusCode::FAILURE;

  return Error("Initialization failure");
}
