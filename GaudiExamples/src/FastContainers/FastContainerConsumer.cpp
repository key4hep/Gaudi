// $Id: FastContainerConsumer.cpp,v 1.2 2006/11/29 18:47:33 hmd Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h" 

// local
#include "FastContainerConsumer.h"

//-----------------------------------------------------------------------------
// Implementation file for class : FastContainerConsumer
//
// 2006-05-03 : Marco Clemencic
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( FastContainerConsumer );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
FastContainerConsumer::FastContainerConsumer( const std::string& name,
                                              ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator ),
    m_container(NULL)
{
  declareProperty("Container", m_containerLocation = "MyContainer");
}

//=============================================================================
// Destructor
//=============================================================================
FastContainerConsumer::~FastContainerConsumer() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode FastContainerConsumer::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;
  
  m_container = getFastContainer<internal_type>(m_containerLocation);

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode FastContainerConsumer::execute() {

  debug() << "==> Execute" << endmsg;

  debug() << "container size = " << m_container->size() << endmsg;
  if ( m_container->size() != 3 ){
    throw GaudiException("the size of the container should be 3",
                         "FastContainerConsumer::execute",
                         StatusCode::FAILURE);
  }

  container_type::iterator i = m_container->begin();
  info() << "event " << **(i++) << endmsg;
  info() << "      " << **(i++) << endmsg;
  info() << "      " << **(i) << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode FastContainerConsumer::finalize() {

  debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================
