#ifndef LOOPALG_H
#define LOOPALG_H 1

// Include files
#include "GaudiKernel/Algorithm.h"
class LoopAlg: public Algorithm {
public:
  // Constructor of this form must be provided
  LoopAlg(const std::string& name, ISvcLocator* pSvcLocator);

  // Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
};

# endif    // THIST_HISTOALGORITHM_H
