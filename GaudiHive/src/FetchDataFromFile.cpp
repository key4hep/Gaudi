#include "GaudiAlg/GaudiAlgorithm.h"

namespace Gaudi {
  namespace Hive {
    class FetchDataFromFile final: public GaudiAlgorithm {
      StringArrayProperty m_dataKeys;
      std::vector<DataObjectHandle<DataObject> *> m_outputHandles;
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
          int i = 0;
          for (auto k: m_dataKeys.value()) {
            debug() << "adding data key " << k << endmsg;
            evtSvc()->addPreLoadItem(k);
            m_outputHandles.push_back( new DataObjectHandle<DataObject>( k, Gaudi::DataHandle::Writer, this ) );
            declareProperty("dummy_out_" + std::to_string(i), *(m_outputHandles.back()) );
            i++;
          }
        }
        return sc;
      }
      StatusCode execute() override {
        return evtSvc()->preLoad();
      }
      ~FetchDataFromFile() override {
        for (auto& h: m_outputHandles) {
          delete h;
        }
      }
    };
  }
}
using Gaudi::Hive::FetchDataFromFile;
DECLARE_COMPONENT(FetchDataFromFile)
