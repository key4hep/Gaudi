#include "GaudiAlg/GaudiAlgorithm.h"

namespace Gaudi
{
  namespace Hive
  {
    class FetchDataFromFile final : public Algorithm
    {
    public:
      FetchDataFromFile( const std::string& name, ISvcLocator* pSvcLocator ) : Algorithm( name, pSvcLocator )
      {
        // make sure this algorithm is seen as reentrant by Gaudi
        this->setProperty( "Cardinality", 0 );
      }
      StatusCode initialize() override
      {
        StatusCode sc = Algorithm::initialize();
        if ( sc ) {
          // this is a hack to reuse the automatic dependencies declaration
          for ( auto k : m_dataKeys ) {
            addDynamicEventOutput( k );
          }
        }
        return sc;
      }
      StatusCode start() override
      {
        StatusCode sc = Algorithm::start();
        if ( sc ) {
          for ( const auto& k : eventOutputKeys() ) {
            if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) ) debug() << "adding data key " << k << endmsg;
            evtSvc()->addPreLoadItem( k.key() );
          }
        }
        return sc;
      }
      StatusCode execute() override { return evtSvc()->preLoad(); }

    private:
      Gaudi::Property<std::vector<std::string>> m_dataKeys{
          this, "DataKeys", {}, "list of objects to be read from file"};
    };
  }
}
using Gaudi::Hive::FetchDataFromFile;
DECLARE_COMPONENT( FetchDataFromFile )
