#include "GaudiKernel/Algorithm.h"

namespace GaudiTesting
{
  namespace JIRA
  {

    /** Special service to expose GAUDI-1185.
     */
    class GAUDI_1185 : public Algorithm
    {
    public:
      using Algorithm::Algorithm;

      StatusCode initialize() override
      {
        StatusCode sc = Algorithm::initialize();
        if ( !sc ) return sc;

        auto level = msgLevel();
        always() << "Initial msgLevel(): " << int( level ) << endmsg;
        always() << "Current msgLevel(): " << int( msgLevel() ) << endmsg;

        return sc;
      }
      StatusCode execute() override { return StatusCode::SUCCESS; }
    };

    DECLARE_COMPONENT( GAUDI_1185 )
  }
}
