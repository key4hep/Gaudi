#ifndef HISTOGRAMS_NTupleAlgorithm_H
#define HISTOGRAMS_NTupleAlgorithm_H 1

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/NTuple.h"

// Forward declarations

class NTupleAlgorithm : public Algorithm
{

public:
  // Constructor of this form must be provided
  NTupleAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );

  // Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  NTuple::Tuple* m_tuple1 = nullptr;
  NTuple::Tuple* m_tuple2 = nullptr;

  NTuple::Item<long>  m_ntrk;
  NTuple::Item<float> m_energy;
  // Items for the column wise n-tuple
  NTuple::Array<long>  m_iNumbers;
  NTuple::Array<float> m_fNumbers;
  NTuple::Item<long>   m_n;
};

#endif // HISTOGRAMS_NTupleAlgorithm_H
