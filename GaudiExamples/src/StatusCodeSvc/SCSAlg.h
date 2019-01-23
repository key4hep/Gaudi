#ifndef SCS_SCSALG_H
#define SCS_SCSALG_H

#include "GaudiKernel/Algorithm.h"

/**
 * @brief  an algorithm to test the StatusCodeSvc
 * @author Charles Leggett, Marco Clemencic
 */
class SCSAlg : public Algorithm
{

public:
  using Algorithm::Algorithm;
  StatusCode execute() override;

  StatusCode test();

  void fncChecked();
  void fncUnchecked();
  void fncUnchecked2();
  void fncIgnored();

private:
};

#endif
