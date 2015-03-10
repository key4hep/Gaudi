#ifndef SRC_GPUTESTALG_H
#define SRC_GPUTESTALG_H 1
// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"


/** @class GPUTestAlg GPUTestAlg.h src/GPUTestAlg.h
  * 
  * 
  * @author Illya Shapoval
  * @date 10/03/2015
  */
class GPUTestAlg: public GaudiAlgorithm {
public:
  /// Standard constructor
  GPUTestAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~GPUTestAlg(); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization
protected:
private:
};

#endif // SRC_GPUTESTALG_H
