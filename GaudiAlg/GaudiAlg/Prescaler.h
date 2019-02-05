
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/Property.h"

class GAUDI_API Prescaler : public GaudiAlgorithm {

public:
  /**
   ** Constructor(s)
   **/
  Prescaler( std::string name, ISvcLocator* pSvcLocator );

  /*****************************
   ** Public Function Members **
   *****************************/

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  /**
   ** Percentage of events that should be passed
   **/
  Gaudi::CheckedProperty<double> m_percentPass{this, "PercentPass", 100.0,
                                               "percentage of events that should be passed"};

  /**
   ** Number of events passed
   **/
  int m_pass = 0;

  /**
   ** Number of events seen
   **/
  int m_seen = 0;
};
