// Include files
#include <vector>

 // from Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/AnyDataWrapper.h"

// local
#include "AnyDataGetAlgorithm.h"

//-----------------------------------------------------------------------------
// Implementation file for class : AnyDataGetAlgorithm
//
// 2016-05-26 : Roel Aaij
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT_WITH_ID( AnyDataGetAlgorithm<int>, "AnyDataGetAlgorithm_Int" )
DECLARE_COMPONENT_WITH_ID( AnyDataGetAlgorithm<std::vector<int>>, "AnyDataGetAlgorithm_VectorInt" )

namespace {
   using std::vector;
}

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
template <class T>
AnyDataGetAlgorithm<T>::AnyDataGetAlgorithm( const std::string& name,
                                          ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
{
   declareProperty("Location", m_location);
}
//=============================================================================
// Destructor
//=============================================================================
template <class T>
AnyDataGetAlgorithm<T>::~AnyDataGetAlgorithm() {}

//=============================================================================
// Initialization
//=============================================================================
template <class T>
StatusCode AnyDataGetAlgorithm<T>::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
template <class T>
StatusCode AnyDataGetAlgorithm<T>::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

  auto base = getIfExists<AnyDataWrapperBase>(m_location);
  if (base) {
     info() << "Got base from " << m_location << endmsg;
  }
  const auto i = dynamic_cast<const AnyDataWrapper<T>*>(base);
  if (i) {
     info() << "Got " << System::typeinfoName(typeid(T)) << " from " << m_location
            << ": " << i->getData() << endmsg;
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
template <class T>
StatusCode AnyDataGetAlgorithm<T>::finalize() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}
