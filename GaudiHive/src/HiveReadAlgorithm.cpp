#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/DataStoreItem.h"

class GAUDI_API HiveReadAlgorithm: public GaudiAlgorithm {
public:
  HiveReadAlgorithm( const std::string& name, ISvcLocator* pSvcLocator ) :  GaudiAlgorithm(name, pSvcLocator) {}
  virtual ~HiveReadAlgorithm() {}
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize() {return StatusCode::SUCCESS;}
};

DECLARE_ALGORITHM_FACTORY(HiveReadAlgorithm)

StatusCode HiveReadAlgorithm::initialize()
{
  return evtSvc()->addPreLoadItem(DataStoreItem("/Event",99));
}

StatusCode HiveReadAlgorithm::execute()
{
  info() << "Running now for event " << getContext()->m_evt_num << endmsg;
  return evtSvc()->preLoad();
}
