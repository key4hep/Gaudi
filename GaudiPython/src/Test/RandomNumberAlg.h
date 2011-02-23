// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPython/src/Test/RandomNumberAlg.h,v 1.3 2008/10/27 19:22:21 marcocle Exp $	//
#ifndef GAUDIEXAMPLES_RANDOMNUMBERALG_H
#define GAUDIEXAMPLES_RANDOMNUMBERALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/RndmGenerators.h"

namespace AIDA {
  class IHistogram1D;
  class IHistogram2D;
}


/** @class RandomNumberAlg
    A small algorithm class using the random number service

    @author M.Frank
    @date 1999
*/
class RandomNumberAlg : public Algorithm     {
 protected:
  /// Allocate wrapper for random number generator
  Rndm::Numbers  m_gaussNumbers;
  Rndm::Numbers  m_poissonNumbers;
  Rndm::Numbers  m_expNumbers;

  AIDA::IHistogram1D*  m_gaussHisto;
  AIDA::IHistogram2D*  m_gauss2Histo;
  AIDA::IHistogram1D*  m_poissonHisto;
  AIDA::IHistogram1D*  m_expHisto;

public:
  /// Constructor: A constructor of this form must be provided.
  RandomNumberAlg(const std::string& name, ISvcLocator* pSvcLocator);
  /// Standard Destructor
  virtual ~RandomNumberAlg();
  /// Customized initialisation
  virtual StatusCode initialize();
  /// Customized finalisation
  virtual StatusCode finalize();
  /// Event callback
  virtual StatusCode execute();
};

#endif // GAUDIEXAMPLES_RANDOMNUMBERALG_H
