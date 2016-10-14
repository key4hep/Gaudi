#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/DataStoreItem.h"

class GAUDI_API HiveReadAlgorithm: public GaudiAlgorithm {
public:
  HiveReadAlgorithm( const std::string& name, ISvcLocator* pSvcLocator ) :  GaudiAlgorithm(name, pSvcLocator) {}
  ~HiveReadAlgorithm() override {}
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override {return StatusCode::SUCCESS;}
};

DECLARE_ALGORITHM_FACTORY(HiveReadAlgorithm)

StatusCode HiveReadAlgorithm::initialize(){
    return evtSvc()->addPreLoadItem(DataStoreItem("/Event",99));
}

StatusCode HiveReadAlgorithm::execute()
{
  const EventContext* ctxt = getContext();
  if (ctxt){
    info() << "Running now for event " << ctxt->evt() << endmsg;
  }
  return evtSvc()->preLoad();
}
