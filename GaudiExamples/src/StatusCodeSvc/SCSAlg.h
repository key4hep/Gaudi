#ifndef SCS_SCSALG_H
#define SCS_SCSALG_H

//<<<<<< INCLUDES                                                       >>>>>>
#include "GaudiKernel/Algorithm.h"
//<<<<<< CLASS DECLARATIONS                                             >>>>>>
/** @class ColorMgs
 * @brief  an algorithm to test the StatusCodeSvc
 * @author Charles Leggett
 */

class SCSAlg : public Algorithm {

public:
  SCSAlg (const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

  StatusCode test();

  void fncChecked();
  void fncUnchecked();
  void fncUnchecked2();
  
private:

};

#endif // SCS_SCSALG_H
