#ifndef GAUDIEXAMPLES_REENTALG_H
#define GAUDIEXAMPLES_REENTALG_H

#include <Gaudi/Algorithm.h>

/** @class ReEntAlg
 * @brief  an algorithm to test reentrant Algorithms
 * @author Charles Leggett
 */
class ReEntAlg : public Gaudi::Algorithm
{

public:
  ReEntAlg( const std::string& name, ISvcLocator* pSvcLocator );
  StatusCode initialize() override;
  StatusCode execute( const EventContext& ctx ) const override;
  StatusCode finalize() override;

private:
  Gaudi::Property<unsigned int> m_sleep{this, "SleepFor", 1000, "ms to sleep in execute"};
};

#endif // GAUDIEXAMPLES_REENTALG_H
