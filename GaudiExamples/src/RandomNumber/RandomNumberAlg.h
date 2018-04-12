#ifndef GAUDIEXAMPLES_RANDOMNUMBERALG_H
#define GAUDIEXAMPLES_RANDOMNUMBERALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/RndmGenerators.h"

/** @class RandomNumberAlg
    A small algorithm class using the random number service

    @author M.Frank
    @date 1999
*/
class RandomNumberAlg : public Algorithm
{
protected:
  /// Allocate wrapper for random number generator
  Rndm::Numbers m_numbers;
  /// Initial seed to fill deterministic random numbers
  uint32_t m_initial;
  /// Pointer to N-tuple
  NTuple::Tuple* m_ntuple;
  /// N-tuple items
  NTuple::Item<int>      m_int;
  NTuple::Item<uint32_t> m_deter;
  NTuple::Item<float>    m_gauss;
  NTuple::Item<float>    m_exponential;
  NTuple::Item<float>    m_poisson;

public:
  /// Constructor: A constructor of this form must be provided.
  RandomNumberAlg( const std::string& name, ISvcLocator* pSvcLocator );
  /// Standard Destructor
  ~RandomNumberAlg() override;
  /// Customized initialisation
  StatusCode initialize() override;
  /// Customized finalisation
  StatusCode finalize() override;
  /// Event callback
  StatusCode execute() override;
};

#endif // GAUDIEXAMPLES_RANDOMNUMBERALG_H
