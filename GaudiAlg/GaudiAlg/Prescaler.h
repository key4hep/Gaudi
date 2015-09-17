
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/Property.h"

class GAUDI_API Prescaler: public GaudiAlgorithm
{

public:

    /**
     ** Constructor(s)
     **/
    Prescaler( std::string name, ISvcLocator* pSvcLocator );

    /**
     ** Destructor
     **/
    ~Prescaler( ) override = default;

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
    DoubleProperty m_percentPass;

    /**
     ** Number of events passed
     **/
    int m_pass = 0;

    /**
     ** Number of events seen
     **/
    int m_seen = 0;
};
