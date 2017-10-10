#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/ThreadLocalContext.h"

class GAUDI_API HiveReadAlgorithm : public GaudiAlgorithm
{
public:
  HiveReadAlgorithm( const std::string& name, ISvcLocator* pSvcLocator ) : GaudiAlgorithm( name, pSvcLocator ) {}
  ~HiveReadAlgorithm() override {}
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override { return StatusCode::SUCCESS; }
};

DECLARE_ALGORITHM_FACTORY( HiveReadAlgorithm )

StatusCode HiveReadAlgorithm::initialize() { return evtSvc()->addPreLoadItem( DataStoreItem( "/Event", 99 ) ); }

StatusCode HiveReadAlgorithm::execute()
{
  info() << "Running now for event " << Gaudi::Hive::currentContext().evt() << endmsg;
  return evtSvc()->preLoad();
}
