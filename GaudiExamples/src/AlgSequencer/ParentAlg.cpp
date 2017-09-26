// Include files
#include "ParentAlg.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/MsgStream.h"

// Static Factory declaration

DECLARE_COMPONENT( ParentAlg )

// Constructor
//------------------------------------------------------------------------------
ParentAlg::ParentAlg( const std::string& name, ISvcLocator* ploc ) : GaudiAlgorithm( name, ploc )
{
  //------------------------------------------------------------------------------
}

//------------------------------------------------------------------------------
StatusCode ParentAlg::initialize()
{
  //------------------------------------------------------------------------------
  StatusCode sc;

  info() << "creating sub-algorithms...." << endmsg;

  sc = createSubAlgorithm( "SubAlg", "SubAlg1", m_subalg1 );
  if ( sc.isFailure() ) return Error( "Error creating Sub-Algorithm SubAlg1", sc );

  sc = createSubAlgorithm( "SubAlg", "SubAlg2", m_subalg2 );
  if ( sc.isFailure() ) return Error( "Error creating Sub-Algorithm SubAlg2", sc );

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode ParentAlg::execute()
{
  //------------------------------------------------------------------------------
  StatusCode sc;
  info() << "executing...." << endmsg;

  for ( auto alg : ( *subAlgorithms() ) ) {
    sc = alg->execute();
    if ( sc.isFailure() ) {
      error() << "Error executing Sub-Algorithm" << alg->name() << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode ParentAlg::finalize()
{
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;
  return StatusCode::SUCCESS;
}
