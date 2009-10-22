// $Id: FastContainerProducer.cpp,v 1.2 2006/11/29 18:47:33 hmd Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h" 

// local
#include "FastContainerProducer.h"

//-----------------------------------------------------------------------------
// Implementation file for class : FastContainerProducer
//
// 2006-05-03 : Marco Clemencic
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( FastContainerProducer );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
FastContainerProducer::FastContainerProducer( const std::string& name,
                                              ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator ),
    m_container(NULL)
{
  declareProperty("Container", m_containerLocation = "MyContainer");
}
//=============================================================================
// Destructor
//=============================================================================
FastContainerProducer::~FastContainerProducer() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode FastContainerProducer::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;
  
  m_container = getFastContainer<internal_type>(m_containerLocation,5);

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode FastContainerProducer::execute() {
  static int evt = 0;

  debug() << "==> Execute" << endmsg;

  debug() << "container size = " << m_container->size() << endmsg;

  // Create a new object with the default constructor.
  internal_type *dbl = m_container->New();
  *dbl = ++evt;

  // Instantiate a new object using the copy constructor.
  m_container->Add(2.5 * evt);

  // Instantiate the new object with a custom constructor.
  new(m_container->NewPointer()) internal_type(1.5 * evt);

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode FastContainerProducer::finalize() {

  debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================
