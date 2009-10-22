// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiExamples/src/RandomNumber/RandomNumberAlg.h,v 1.3 2007/09/28 11:48:17 marcocle Exp $	//
#ifndef GAUDIEXAMPLES_RANDOMNUMBERALG_H
#define GAUDIEXAMPLES_RANDOMNUMBERALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h" 
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/NTuple.h"

/** @class RandomNumberAlg 
    A small algorithm class using the random number service

    @author M.Frank
    @date 1999
*/
class RandomNumberAlg : public Algorithm     {
 protected:
  /// Allocate wrapper for random number generator
  Rndm::Numbers  m_numbers;
  /// Pointer to N-tuple
  NTuple::Tuple*       m_ntuple;
  /// N-tuple items 
  NTuple::Item<int>    m_int;
  NTuple::Item<float>  m_gauss;
  NTuple::Item<float>  m_exponential;
  NTuple::Item<float>  m_poisson;

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
