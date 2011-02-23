// $Id: NTupleAlgorithm.h,v 1.3 2007/09/28 11:48:17 marcocle Exp $
#ifndef HISTOGRAMS_NTupleAlgorithm_H
#define HISTOGRAMS_NTupleAlgorithm_H 1

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/NTuple.h"

// Forward declarations

class NTupleAlgorithm : public Algorithm {

public:
  // Constructor of this form must be provided
  NTupleAlgorithm(const std::string& name, ISvcLocator* pSvcLocator); 

  // Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:

  NTuple::Tuple*  m_tuple1;
  NTuple::Tuple*  m_tuple2;

  NTuple::Item<int>               m_ntrk;
  NTuple::Item<float>             m_energy;
  // Items for the column wise n-tuple
  NTuple::Array<int>              m_iNumbers;
  NTuple::Array<float>            m_fNumbers;
  NTuple::Item<int>               m_n;
};


# endif    // HISTOGRAMS_NTupleAlgorithm_H
