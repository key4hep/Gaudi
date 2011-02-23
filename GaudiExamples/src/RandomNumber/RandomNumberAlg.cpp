//$Id: RandomNumbetrAlg.cpp,v 1.16 2007/09/28 11:48:17 marcocle Exp $	//

// Framework include files
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IRndmGen.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/SmartDataPtr.h"

#include "AIDA/IHistogram1D.h"
using AIDA::IHistogram1D;

// Example related include files
#include "RandomNumberAlg.h"

DECLARE_ALGORITHM_FACTORY(RandomNumberAlg)

/**  Algorithm parameters which can be set at run time must be declared.
     This should be done in the constructor.
*/
RandomNumberAlg::RandomNumberAlg(const std::string& name, ISvcLocator* pSvcLocator)
: Algorithm(name, pSvcLocator)    {
}

// Standard destructor
RandomNumberAlg::~RandomNumberAlg()   {
  std::cout << "Destructor Called for " << name() <<std::endl;
}

// The "functional" part of the class: For the EmptyAlgorithm example they do
//  nothing apart from print out info messages.
StatusCode RandomNumberAlg::initialize() {
  MsgStream log(msgSvc(), name());
  // Use the Job options service to set the Algorithm's parameters
  StatusCode status = setProperties();
  //
  // The first example is for purists:
  // Every step is done by hand....tends to become complicated,
  // but shows the usage of the raw interfaces
  //
  // Get random number generator:
  SmartIF<IRndmGen> gen;
  {
    IRndmGen* tmpPtr = 0;
    status = randSvc()->generator( Rndm::Gauss(0.5,0.2), tmpPtr );
    gen = tmpPtr;
  }
  if ( status.isSuccess() )   {
    std::vector<double> numbers;
    gen->shootArray(numbers, 5000).ignore();
    IHistogram1D* his = histoSvc()->book( "1", "Gauss", 40, 0., 3.);
    for ( unsigned int i = 0; i < numbers.size(); i++ )
      his->fill(numbers[i], 1.0);

    for ( int j = 0; j < 5000; j++ )
      his->fill(gen->shoot(), 1.0);
  }

  //
  // Now we do it as proposed for LOCAL usage of the wrapper
  //
  Rndm::Numbers exponential(randSvc(), Rndm::Exponential(0.2));
  if ( exponential ) {
    IHistogram1D* his = histoSvc()->book( "2", "Exponential", 40, 0., 3.);
    for ( long j = 0; j < 5000; j++ )
      his->fill(exponential(), 1.0);
  }
  else {
    return StatusCode::FAILURE;
  }

  //
  // Now we do it as proposed for GLOBAL usage of the wrapper
  // - Initialize the wrapper allocated in the header file
  //
  status = m_numbers.initialize(randSvc(), Rndm::Poisson(0.3));
  if ( !status.isSuccess() ) {
    return status;
  }

  // The GLOBAL wrapper is now initialized and ready for use.
  // The code below could go anywhere. It is only for simplicity
  // in the "initialize" method!
  {
    IHistogram1D* hispoisson = histoSvc()->book( "3", "Poisson", 40, 0., 3.);
    for ( long j = 0; j < 5000; j++ )
      hispoisson->fill(m_numbers(), 1.0);
  }
  //
  // Test Gaussian Tail distribution
  //
  Rndm::Numbers gaussiantail(randSvc(), Rndm::GaussianTail(20., 10.));
  if ( gaussiantail ) {
    IHistogram1D* his = histoSvc()->book( "4", "GaussianTail", 50, 0., 50.);
    for ( long j = 0; j < 50009; j++ )
      his->fill(gaussiantail(), 1.0);
  }
  else {
    return StatusCode::FAILURE;
  }

  // Book N-tuple

  m_ntuple = ntupleSvc()->book ("/NTUPLES/FILE1/100", CLID_RowWiseTuple, "Hello World");
  if ( m_ntuple )    {
    status = m_ntuple->addItem ("Event#",  m_int);
    status = m_ntuple->addItem ("Gauss",   m_gauss);
    status = m_ntuple->addItem ("Exp",     m_exponential);
    status = m_ntuple->addItem ("Poisson", m_poisson);
  }
  return status;
}

StatusCode RandomNumberAlg::execute()   {
  MsgStream log(msgSvc(), name());
  StatusCode status;
  static int count = 0;

  Rndm::Numbers gauss(randSvc(),       Rndm::Gauss(0.5,0.2));
  Rndm::Numbers exponential(randSvc(), Rndm::Exponential(0.2));
  Rndm::Numbers poisson(randSvc(),     Rndm::Poisson(0.3));

  m_int         = ++count;
  m_gauss       = (float)gauss();
  m_exponential = (float)exponential();
  m_poisson     = (float)poisson();

  status = m_ntuple->write();
  if ( !status.isSuccess() )   {
    log << MSG::ERROR << "Cannot fill NTuple" << endmsg;
  }
  return StatusCode::SUCCESS;
}

StatusCode RandomNumberAlg::finalize()   {
  m_numbers.finalize().ignore();
  return StatusCode::SUCCESS;
}

