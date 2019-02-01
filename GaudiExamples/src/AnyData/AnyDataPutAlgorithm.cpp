// Include files
#include <string>
#include <vector>

// from Gaudi
#include "GaudiKernel/AnyDataWrapper.h"

// local
#include "AnyDataPutAlgorithm.h"

//-----------------------------------------------------------------------------
// Implementation file for class : AnyDataPutAlgorithm
//
// 2016-05-26 : Roel Aaij
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( AnyDataPutAlgorithm )

namespace {
  using std::vector;
}

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
AnyDataPutAlgorithm::AnyDataPutAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
    : GaudiAlgorithm( name, pSvcLocator ) {
  for ( int i = 0; i < 100; i++ ) {
    m_id_vec.emplace_back( "/Event/Test/Ids" + std::to_string( i ), Gaudi::DataHandle::Writer, this );
  }
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode AnyDataPutAlgorithm::execute() {

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

  auto i = std::make_unique<AnyDataWrapper<int>>( 0 );
  auto j = std::make_unique<AnyDataWrapper<vector<int>>>( std::vector<int>{0, 1, 2, 3} );

  put( std::move( i ), m_loc + "/One" );
  put( std::move( j ), m_loc + "/Two" );

  m_ids.put( vector<int>( {42, 84} ) );

  for ( int i = 0; i < 100; ++i ) m_id_vec[i].put( std::move( i ) );

  return StatusCode::SUCCESS;
}
