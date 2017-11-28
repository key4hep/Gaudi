#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"

class GAUDI_API EventCounter : public Algorithm
{
public:
  /**
   ** Constructor(s)
   **/
  EventCounter( const std::string& name, ISvcLocator* pSvcLocator );

  /*****************************
   ** Public Function Members **
   *****************************/

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  /**************************
   ** Private Data Members **
   **************************/

  /**
   ** The frequency with which the number of events
   ** should be reported. The default is 1, corresponding
   ** to every event.
   **/
  Gaudi::CheckedProperty<int> m_frequency{this, "Frequency", 1};

  /**
   ** The number of events skipped since the last time
   ** the count was reported.
   **/
  int m_skip = 0;

  /**
   ** The total events seen.
   **/
  int m_total = 0;
};
