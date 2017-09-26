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
  SCSAlg( const std::string& name, ISvcLocator* pSvcLocator );
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

  StatusCode test();

  void fncChecked();
  void fncUnchecked();
  void fncUnchecked2();
  void fncIgnored();

private:
};

#endif
