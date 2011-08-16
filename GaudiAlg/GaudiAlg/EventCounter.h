#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"

class GAUDI_API EventCounter: public Algorithm {
public:

    /**
     ** Constructor(s)
     **/
    EventCounter( const std::string& name, ISvcLocator* pSvcLocator );

    /**
     ** Destructor
     **/
    virtual ~EventCounter( );

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
     ** The frequency with which the number of events
     ** should be reported. The default is 1, corresponding
     ** to every event.
     **/
    IntegerProperty m_frequency;

    /**
     ** The number of events skipped since the last time
     ** the count was reported.
     **/
    int m_skip;

    /**
     ** The total events seen.
     **/
    int m_total;
};
