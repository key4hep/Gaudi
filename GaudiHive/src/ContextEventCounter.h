#ifndef SRC_CONTEXTEVENTCOUNTER_H
#define SRC_CONTEXTEVENTCOUNTER_H 1
// Include files
// from Gaudi
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ContextSpecificPtr.h"

/** @class ContextEventCounter ContextEventCounter.h src/ContextEventCounter.h
 *
 *
 * @author Marco Clemencic
 * @date 27/10/2013
 */
class ContextEventCounterPtr final : public Algorithm {
public:
  using Algorithm::Algorithm;     /// Standard constructor
  StatusCode execute() override;  ///< Algorithm execution
  StatusCode finalize() override; ///< Algorithm finalization
private:
  Gaudi::Hive::ContextSpecificPtr<int> m_ctxtSpecCounter;
};

class ContextEventCounterData final : public Algorithm {
public:
  using Algorithm::Algorithm;     /// Standard constructor
  StatusCode execute() override;  ///< Algorithm execution
  StatusCode finalize() override; ///< Algorithm finalization
private:
  Gaudi::Hive::ContextSpecificData<int> m_ctxtSpecCounter;
};

#endif // SRC_CONTEXTEVENTCOUNTER_H
