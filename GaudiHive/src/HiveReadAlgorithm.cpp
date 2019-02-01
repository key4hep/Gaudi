#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/ThreadLocalContext.h"

class GAUDI_API HiveReadAlgorithm : public GaudiAlgorithm {
public:
  using GaudiAlgorithm::GaudiAlgorithm;
  StatusCode initialize() override;
  StatusCode execute() override;
};

DECLARE_COMPONENT( HiveReadAlgorithm )

StatusCode HiveReadAlgorithm::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize();
  if ( !sc ) return sc;
  return evtSvc()->addPreLoadItem( DataStoreItem( "/Event", 99 ) );
}

StatusCode HiveReadAlgorithm::execute() {
  info() << "Running now for event " << Gaudi::Hive::currentContext().evt() << endmsg;
  return evtSvc()->preLoad();
}
