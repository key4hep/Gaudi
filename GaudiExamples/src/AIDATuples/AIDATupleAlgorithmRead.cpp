// Include files
#include "AIDATupleAlgorithmRead.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IAIDATupleSvc.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartDataPtr.h"

// Static factory declaration

DECLARE_COMPONENT( AIDATupleAlgorithmRead )

//--------------------------------------------------------
AIDATupleAlgorithmRead::AIDATupleAlgorithmRead( const std::string& name, ISvcLocator* pSvcLocator )
    : Algorithm( name, pSvcLocator )
//--------------------------------------------------------
{
  tuple = 0;
}

//--------------------------------------
StatusCode AIDATupleAlgorithmRead::initialize()
//--------------------------------------
{
  StatusCode status;
  info() << "Initializing..." << endmsg;

  // status = atupleSvc()->myTest();

  // Book N-tuple 1

  status = atupleSvc()->retrieveObject( "MyTuples/1", tuple );

  info() << "Tuple: " << long( tuple ) << endmsg;
  info() << "Number of rows of the Tuple: " << tuple->rows() << endmsg;
  if ( status.isSuccess() ) {
    info() << "Registering Tuple" << endmsg;
    status = atupleSvc()->registerObject( "MyTuples", "1", tuple );
  }

  info() << "Finished booking NTuples" << endmsg;

  return status;
}

//-----------------------------------
StatusCode AIDATupleAlgorithmRead::execute()
//-----------------------------------
{

  // StatusCode status;
  info() << "Executing..." << endmsg;

  int i_px   = tuple->findColumn( "px" );
  int i_py   = tuple->findColumn( "py" );
  int i_pz   = tuple->findColumn( "pz" );
  int i_mass = tuple->findColumn( "mass" );

  float px;
  float py;
  float pz;
  float mass;

  tuple->start();
  int i = 0;
  while ( tuple->next() ) {
    i++;
    info() << "COLUMN " << i;
    px = tuple->getFloat( i_px );
    info() << " px: " << px;
    py = tuple->getFloat( i_py );
    info() << " py: " << py;
    pz = tuple->getFloat( i_pz );
    info() << " pz: " << pz;
    mass = tuple->getFloat( i_mass );
    info() << " mass: " << mass << endmsg;
  }

  info() << "Reading mass>1" << endmsg;
  atupleSvc()->setCriteria( tuple, "mass>1" );

  tuple->start();
  i = 0;
  while ( tuple->next() ) {
    i++;
    info() << "COLUMN " << i;
    px = tuple->getFloat( i_px );
    info() << " px: " << px;
    py = tuple->getFloat( i_py );
    info() << " py: " << py;
    pz = tuple->getFloat( i_pz );
    info() << " pz: " << pz;
    mass = tuple->getFloat( i_mass );
    info() << " mass: " << mass << endmsg;
  }

  return StatusCode::SUCCESS;
}

//------------------------------------
StatusCode AIDATupleAlgorithmRead::finalize()
//------------------------------------
{
  info() << "Finalizing..." << endmsg;

  return StatusCode::SUCCESS;
}
