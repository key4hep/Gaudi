// $Id: AIDATupleAlgorithmWrite.cpp,v 1.3 2006/11/27 09:53:05 hmd Exp $
// Include files
#include "AIDATupleAlgorithmWrite.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IAIDATupleSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/DataObject.h"

#include "CLHEP/Random/RandGauss.h"
#include "CLHEP/Random/DRand48Engine.h"

// Handle CLHEP 2.0.x move to CLHEP namespace
namespace CLHEP { }
using namespace CLHEP;

//--------------------------------------------------------
AIDATupleAlgorithmWrite::AIDATupleAlgorithmWrite(const std::string& name,
                                 ISvcLocator* pSvcLocator)
                :Algorithm(name, pSvcLocator)
//--------------------------------------------------------
{
  tuple = 0;
}


//--------------------------------------
StatusCode AIDATupleAlgorithmWrite::initialize()
//--------------------------------------
{
  //StatusCode status;
  MsgStream log( msgSvc(), name() );
  log << MSG::INFO << "Initializing..." << endmsg;

  //status = atupleSvc()->myTest();

  SmartDataPtr<ITuple> nt1(atupleSvc(),"MyTuples/1");
  if ( nt1 ) {
    tuple = nt1;
  }
  else {
    std::string columns =  "float px; float py; float pz; float mass";
    tuple = atupleSvc()->book ("MyTuples/1", "example tuple", columns);

    if ( !tuple ) { // did not manage to book the N tuple....
		log << MSG::ERROR << "Cannot book N-tuple:" << long(tuple) << endmsg;
		return StatusCode::FAILURE;
    }
  }

  log << MSG::INFO << "Finished booking NTuples" << endmsg;
  return StatusCode::SUCCESS;
}


//-----------------------------------
StatusCode AIDATupleAlgorithmWrite::execute()
//-----------------------------------
{

  //StatusCode status;
  MsgStream log( msgSvc(), name() );
  log << MSG::INFO << "Executing..." << endmsg;

  DRand48Engine randomEngine;
  RandGauss rBeamEnergy( randomEngine, 90, 5 );
  RandGauss rTracksSpread( randomEngine, 0, 2 );
  RandGauss rMomentum( randomEngine, 0, 3 );
  RandGauss rMass( randomEngine, 1, 0.1 );

  int i_px = tuple->findColumn( "px" );
  int i_py = tuple->findColumn( "py" );
  int i_pz = tuple->findColumn( "pz" );
  int i_mass = tuple->findColumn( "mass" );

  for( int i=0; i<1000; i++ ) {
    tuple->fill( i_px, rMomentum.fire() );
    tuple->fill( i_py, rMomentum.fire() );
    tuple->fill( i_pz, rMomentum.fire() );
    tuple->fill( i_mass, rMass.fire() );
    tuple->addRow();
  }

   log << MSG::INFO << "Filled the tuple with " << tuple->rows() << " rows" << endmsg;
  return StatusCode::SUCCESS;
}


//------------------------------------
StatusCode AIDATupleAlgorithmWrite::finalize()
//------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "Finalizing..." << endmsg;

	return StatusCode::SUCCESS;
}

// Static factory declaration
DECLARE_ALGORITHM_FACTORY(AIDATupleAlgorithmWrite)
