#include "GaudiAlg/GaudiAlgorithm.h"

namespace Gaudi {
  namespace Hive {
    class FetchDataFromFile final: public GaudiAlgorithm {
      StringArrayProperty m_dataKeys;
      std::vector<DataObjectHandle<DataObject>> m_outputHandles;
    public:
      FetchDataFromFile(const std::string& name, ISvcLocator* svcLoc):
        GaudiAlgorithm(name, svcLoc)
      {
        declareProperty("DataKeys", m_dataKeys, "list of objects to be read from file");
      }
      StatusCode initialize() override {
        StatusCode sc = GaudiAlgorithm::initialize();
        if (sc) {
          // this is a hack to reuse the automatic dependencies declaration
          for (auto k: m_dataKeys.value()) {
            debug() << "adding data key " << k << endmsg;
            evtSvc()->addPreLoadItem(k);
            // note: the DataObjectHandleBase constructor will 'declare' the handle
            m_outputHandles.emplace_back( k, Gaudi::DataHandle::Writer, this );
          }
        }
        return sc;
      }
      StatusCode execute() override {
        return evtSvc()->preLoad();
      }
    };
  }
}
using Gaudi::Hive::FetchDataFromFile;
DECLARE_COMPONENT(FetchDataFromFile)
