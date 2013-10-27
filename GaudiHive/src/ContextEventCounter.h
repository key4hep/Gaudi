#ifndef SRC_CONTEXTEVENTCOUNTER_H
#define SRC_CONTEXTEVENTCOUNTER_H 1
// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

#include "GaudiKernel/ContextSpecificPtr.h"

/** @class ContextEventCounter ContextEventCounter.h src/ContextEventCounter.h
  * 
  * 
  * @author Marco Clemencic
  * @date 27/10/2013
  */
class ContextEventCounter: public GaudiAlgorithm {
public:
  /// Standard constructor
  ContextEventCounter(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~ContextEventCounter(); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization
protected:
private:

  Gaudi::Hive::ContextSpecificPtr<int> m_ctxtSpecCounter;
};

#endif // SRC_CONTEXTEVENTCOUNTER_H
