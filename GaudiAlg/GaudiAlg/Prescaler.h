
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/Property.h"

class GAUDI_API Prescaler: public GaudiAlgorithm
{

public:

    /**
     ** Constructor(s)
     **/
    Prescaler( const std::string& name, ISvcLocator* pSvcLocator );

    /**
     ** Destructor
     **/
    virtual ~Prescaler( );

    /*****************************
     ** Public Function Members **
     *****************************/

    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

private:

    /**************************
     ** Private Data Members **
     **************************/

    /**
     ** Percentage of events that should be passed
     **/
    DoubleProperty m_percentPass;

    /**
     ** Number of events passed
     **/
    int m_pass;

    /**
     ** Number of events seen
     **/
    int m_seen;
};
