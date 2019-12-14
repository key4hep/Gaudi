/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// Framework include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/IRndmGen.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/SmartIF.h"

#include "GaudiUtils/QuasiRandom.h"

#include "AIDA/IHistogram1D.h"
using AIDA::IHistogram1D;

// Example related include files
#include "RandomNumberAlg.h"

namespace {
  namespace QuasiRandom = Gaudi::Utils::QuasiRandom;
}

DECLARE_COMPONENT( RandomNumberAlg )

/**  Algorithm parameters which can be set at run time must be declared.
     This should be done in the constructor.
*/
RandomNumberAlg::RandomNumberAlg( const std::string& name, ISvcLocator* pSvcLocator )
    : Algorithm( name, pSvcLocator ) {}

// Standard destructor
RandomNumberAlg::~RandomNumberAlg() {
  // do not print messages if we are created in genconf
  const std::string cmd = System::cmdLineArgs()[0];
  if ( cmd.find( "genconf" ) != std::string::npos ) return;

  std::cout << "Destructor Called for " << name() << std::endl;
}

// The "functional" part of the class: For the EmptyAlgorithm example they do
//  nothing apart from print out info messages.
StatusCode RandomNumberAlg::initialize() {
  // Use the Job options service to set the Algorithm's parameters
  StatusCode status = setProperties();
  //
  // The first example is for purists:
  // Every step is done by hand....tends to become complicated,
  // but shows the usage of the raw interfaces
  //
  // Get random number generator:
  auto gen = randSvc()->generator( Rndm::Gauss( 0.5, 0.2 ) );
  if ( gen ) {
    std::vector<double> numbers;
    gen->shootArray( numbers, 5000 ).ignore();
    IHistogram1D* his = histoSvc()->book( "1", "Gauss", 40, 0., 3. );
    for ( unsigned int i = 0; i < numbers.size(); i++ ) his->fill( numbers[i], 1.0 );

    for ( int j = 0; j < 5000; j++ ) his->fill( gen->shoot(), 1.0 );
  }

  //
  // Now we do it as proposed for LOCAL usage of the wrapper
  //
  Rndm::Numbers exponential( randSvc(), Rndm::Exponential( 0.2 ) );
  if ( exponential ) {
    IHistogram1D* his = histoSvc()->book( "2", "Exponential", 40, 0., 3. );
    for ( long j = 0; j < 5000; j++ ) his->fill( exponential(), 1.0 );
  } else {
    return StatusCode::FAILURE;
  }

  //
  // Now we do it as proposed for GLOBAL usage of the wrapper
  // - Initialize the wrapper allocated in the header file
  //
  status = m_numbers.initialize( randSvc(), Rndm::Poisson( 0.3 ) );
  if ( !status.isSuccess() ) { return status; }

  // The GLOBAL wrapper is now initialized and ready for use.
  // The code below could go anywhere. It is only for simplicity
  // in the "initialize" method!
  {
    IHistogram1D* hispoisson = histoSvc()->book( "3", "Poisson", 40, 0., 3. );
    for ( long j = 0; j < 5000; j++ ) hispoisson->fill( m_numbers(), 1.0 );
  }
  //
  // Test Gaussian Tail distribution
  //
  Rndm::Numbers gaussiantail( randSvc(), Rndm::GaussianTail( 20., 10. ) );
  if ( gaussiantail ) {
    IHistogram1D* his = histoSvc()->book( "4", "GaussianTail", 50, 0., 50. );
    for ( long j = 0; j < 50009; j++ ) his->fill( gaussiantail(), 1.0 );
  } else {
    return StatusCode::FAILURE;
  }

  // Initial randomness for deterministic random numbers
  m_initial = QuasiRandom::mixString( name().size(), name() );

  // Book N-tuple
  m_ntuple = ntupleSvc()->book( "/NTUPLES/FILE1/100", CLID_RowWiseTuple, "Hello World" );
  if ( m_ntuple ) {
    status = m_ntuple->addItem( "Event#", m_int );
    status = m_ntuple->addItem( "DeterInt", m_deter );
    status = m_ntuple->addItem( "Gauss", m_gauss );
    status = m_ntuple->addItem( "Exp", m_exponential );
    status = m_ntuple->addItem( "Poisson", m_poisson );
  }
  return status;
}

StatusCode RandomNumberAlg::execute() {
  StatusCode status;
  static int count = 0;

  Rndm::Numbers gauss( randSvc(), Rndm::Gauss( 0.5, 0.2 ) );
  Rndm::Numbers exponential( randSvc(), Rndm::Exponential( 0.2 ) );
  Rndm::Numbers poisson( randSvc(), Rndm::Poisson( 0.3 ) );

  // Return integer in interval [0, size) from random integer in interval [0, MAX_INT]
  auto scale = []( uint32_t x, uint32_t size ) {
    const uint32_t denom = boost::integer_traits<uint32_t>::const_max / size;
    return x / denom;
  };

  m_int         = ++count;
  m_deter       = scale( QuasiRandom::mix32( m_initial, m_int ), 100 );
  m_gauss       = (float)gauss();
  m_exponential = (float)exponential();
  m_poisson     = (float)poisson();

  status = m_ntuple->write();
  if ( !status.isSuccess() ) { error() << "Cannot fill NTuple" << endmsg; }
  return StatusCode::SUCCESS;
}

StatusCode RandomNumberAlg::finalize() {
  m_numbers.finalize().ignore();
  return StatusCode::SUCCESS;
}
