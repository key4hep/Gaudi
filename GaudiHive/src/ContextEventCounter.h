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
class ContextEventCounterPtr : public GaudiAlgorithm
{
public:
  /// Standard constructor
  ContextEventCounterPtr( const std::string& name, ISvcLocator* pSvcLocator );
  ~ContextEventCounterPtr() override; ///< Destructor

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization
protected:
private:
  Gaudi::Hive::ContextSpecificPtr<int> m_ctxtSpecCounter;
};

class ContextEventCounterData : public GaudiAlgorithm
{
public:
  /// Standard constructor
  ContextEventCounterData( const std::string& name, ISvcLocator* pSvcLocator );
  ~ContextEventCounterData() override; ///< Destructor

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization
protected:
private:
  Gaudi::Hive::ContextSpecificData<int> m_ctxtSpecCounter;
};

#endif // SRC_CONTEXTEVENTCOUNTER_H
