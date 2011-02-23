#include "MTHelloWorld.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "MTMessageSvc.h"

DECLARE_ALGORITHM_FACTORY(MTHelloWorld);

/////////////////////////////////////////////////////////////////////////////

MTHelloWorld::MTHelloWorld(const std::string& name, ISvcLocator* pSvcLocator) :
  Algorithm(name, pSvcLocator), m_myInt(0), m_myBool(0), m_myDouble(0)
{

  // Part 2: Declare the properties
  declareProperty("MyInt", m_myInt);
  declareProperty("MyBool", m_myBool);
  declareProperty("MyDouble", m_myDouble);

  declareProperty("MyStringVec",m_myStringVec);

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode MTHelloWorld::initialize(){

  // Part 1: Get the messaging service, print where you are
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "initialize()" << endmsg;

  MTMessageSvc* tmp_msgSvc = dynamic_cast<MTMessageSvc*> (msgSvc());
  if(tmp_msgSvc != 0) {
    log << MSG::INFO << " Algorithm = " << name() << " is connected to Message Service = "
        << tmp_msgSvc->name() << endmsg;
  }

  // Part 2: Print out the property values
  log << MSG::INFO << "  MyInt =    " << m_myInt << endmsg;
  log << MSG::INFO << "  MyBool =   " << (int)m_myBool << endmsg;
  log << MSG::INFO << "  MyDouble = " << m_myDouble << endmsg;

  for (unsigned int i=0; i<m_myStringVec.size(); i++) {
    log << MSG::INFO << "  MyStringVec[" << i << "] = " << m_myStringVec[i]
	<< endmsg;
  }

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode MTHelloWorld::execute() {

  // Part 1: Get the messaging service, print where you are
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "execute()" << endmsg;

  // Part 1: Print out the different levels of messages
  log << MSG::DEBUG << "A DEBUG message" << endmsg;
  log << MSG::INFO << "An INFO message" << endmsg;
  log << MSG::WARNING << "A WARNING message" << endmsg;
  log << MSG::ERROR << "An ERROR message" << endmsg;
  log << MSG::FATAL << "A FATAL error message" << endmsg;

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode MTHelloWorld::finalize() {

  // Part 1: Get the messaging service, print where you are
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "finalize()" << endmsg;

  return StatusCode::SUCCESS;
}
