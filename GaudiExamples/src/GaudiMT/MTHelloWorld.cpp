#include "MTHelloWorld.h"
#include "GaudiKernel/MsgStream.h"
#include "MTMessageSvc.h"

DECLARE_COMPONENT( MTHelloWorld );

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode MTHelloWorld::initialize()
{

  // Part 1: Get the messaging service, print where you are
  info() << "initialize()" << endmsg;

  MTMessageSvc* tmp_msgSvc = dynamic_cast<MTMessageSvc*>( msgSvc() );
  if ( tmp_msgSvc != 0 ) {
    info() << " Algorithm = " << name() << " is connected to Message Service = " << tmp_msgSvc->name() << endmsg;
  }

  // Part 2: Print out the property values
  info() << "  MyInt =    " << m_myInt.value() << endmsg;
  info() << "  MyBool =   " << (int)m_myBool.value() << endmsg;
  info() << "  MyDouble = " << m_myDouble.value() << endmsg;

  for ( unsigned int i = 0; i < m_myStringVec.size(); i++ ) {
    info() << "  MyStringVec[" << i << "] = " << m_myStringVec[i] << endmsg;
  }

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode MTHelloWorld::execute()
{

  // Part 1: Get the messaging service, print where you are
  info() << "execute()" << endmsg;

  // Part 1: Print out the different levels of messages
  debug() << "A DEBUG message" << endmsg;
  info() << "An INFO message" << endmsg;
  warning() << "A WARNING message" << endmsg;
  error() << "An ERROR message" << endmsg;
  fatal() << "A FATAL error message" << endmsg;

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode MTHelloWorld::finalize()
{

  // Part 1: Get the messaging service, print where you are
  info() << "finalize()" << endmsg;

  return StatusCode::SUCCESS;
}
