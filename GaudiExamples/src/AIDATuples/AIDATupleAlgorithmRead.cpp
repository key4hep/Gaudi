// Include files
#include "AIDATupleAlgorithmRead.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IAIDATupleSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IRegistry.h"

// Static factory declaration

DECLARE_COMPONENT(AIDATupleAlgorithmRead)

//--------------------------------------------------------
AIDATupleAlgorithmRead::AIDATupleAlgorithmRead(const std::string& name,
                                 ISvcLocator* pSvcLocator)
                :Algorithm(name, pSvcLocator)
//--------------------------------------------------------
{
  tuple = 0;
 }


//--------------------------------------
StatusCode AIDATupleAlgorithmRead::initialize()
//--------------------------------------
{
  StatusCode status;
  MsgStream log( msgSvc(), name() );
  log << MSG::INFO << "Initializing..." << endmsg;

  //status = atupleSvc()->myTest();

  //Book N-tuple 1

  status = atupleSvc()->retrieveObject("MyTuples/1", tuple);

  log << MSG::INFO << "Tuple: " << long(tuple) << endmsg;
  log << MSG::INFO << "Number of rows of the Tuple: " << tuple->rows() << endmsg;
  if( status.isSuccess() ) {
    log << MSG::INFO << "Registering Tuple" << endmsg;
    status = atupleSvc()->registerObject("MyTuples", "1", tuple);
  }

  log << MSG::INFO << "Finished booking NTuples" << endmsg;

  return status;
}


//-----------------------------------
StatusCode AIDATupleAlgorithmRead::execute()
//-----------------------------------
{

  //StatusCode status;
  MsgStream log( msgSvc(), name() );
  log << MSG::INFO << "Executing..." << endmsg;

  int i_px = tuple->findColumn( "px" );
  int i_py = tuple->findColumn( "py" );
  int i_pz = tuple->findColumn( "pz" );
  int i_mass = tuple->findColumn( "mass" );

  float px;
  float py;
  float pz;
  float mass;


  tuple->start();
  int i = 0;
  while( tuple->next() ) {
    i++;
    log << MSG::INFO << "COLUMN " << i ;
    px = tuple->getFloat(i_px);
    log << MSG::INFO << " px: " << px ;
    py = tuple->getFloat(i_py);
    log << MSG::INFO << " py: " << py ;
    pz = tuple->getFloat(i_pz);
    log << MSG::INFO << " pz: " << pz ;
    mass = tuple->getFloat(i_mass);
    log << MSG::INFO << " mass: " << mass << endmsg;
  }

  log << MSG::INFO << "Reading mass>1" << endmsg;
  atupleSvc()->setCriteria(tuple,"mass>1");

  tuple->start();
  i = 0;
  while( tuple->next() ) {
    i++;
    log << MSG::INFO << "COLUMN " << i ;
    px = tuple->getFloat(i_px);
    log << MSG::INFO << " px: " << px ;
    py = tuple->getFloat(i_py);
    log << MSG::INFO << " py: " << py ;
    pz = tuple->getFloat(i_pz);
    log << MSG::INFO << " pz: " << pz ;
    mass = tuple->getFloat(i_mass);
    log << MSG::INFO << " mass: " << mass << endmsg;
  }

  return StatusCode::SUCCESS;
}


//------------------------------------
StatusCode AIDATupleAlgorithmRead::finalize()
//------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "Finalizing..." << endmsg;

	return StatusCode::SUCCESS;
}
