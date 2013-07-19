// Framework include files
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IRndmGen.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/SmartDataPtr.h"

#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"

// Example related include files
#include "RandomNumberAlg.h"

DECLARE_COMPONENT(RandomNumberAlg)

/**  Algorithm parameters which can be set at run time must be declared.
     This should be done in the constructor.
*/
RandomNumberAlg::RandomNumberAlg(const std::string& name, ISvcLocator* pSvcLocator)
: Algorithm(name, pSvcLocator)    {
}

// Standard destructor
RandomNumberAlg::~RandomNumberAlg()   {
}

// The "functional" part of the class: For the EmptyAlgorithm example they do
//  nothing apart from print out info messages.
StatusCode RandomNumberAlg::initialize() {
  MsgStream log(msgSvc(), name());
  StatusCode sc;

  // Initilize random number generators
  sc = m_poissonNumbers.initialize(randSvc(), Rndm::Poisson(0.3));
  if ( sc.isFailure() ) return sc;
  sc = m_gaussNumbers.initialize(randSvc(), Rndm::Gauss(0.5,0.2));
  if ( sc.isFailure() ) return sc;
  sc = m_expNumbers.initialize(randSvc(), Rndm::Exponential(0.2));
  if ( sc.isFailure() ) return sc;


  // Initiatize Histograms
  m_gaussHisto   = histoSvc()->book( "simple/1", "Gauss", 40, 0., 3.);
  m_gauss2Histo  = histoSvc()->book( "simple/2", "Gauss 2D", 40, 0., 3., 50, 0., 3.);
  m_expHisto     = histoSvc()->book( "simple/3", "Exponential", 40, 0., 3.);
  m_poissonHisto = histoSvc()->book( "simple/4", "Poisson", 40, 0., 3.);

  return sc;
}

StatusCode RandomNumberAlg::execute()   {
  MsgStream log(msgSvc(), name());
  StatusCode sc;

  m_gaussHisto->fill(m_gaussNumbers(), 1.0 );
  m_gauss2Histo->fill(m_gaussNumbers(), m_gaussNumbers(), 1.0 );
  m_poissonHisto->fill(m_poissonNumbers(), 1.0 );
  m_expHisto->fill(m_expNumbers(), 1.0 );
  return sc;
}

StatusCode RandomNumberAlg::finalize()   {
  return StatusCode::SUCCESS;
}

