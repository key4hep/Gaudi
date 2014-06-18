#include "GaudiKernel/Algorithm.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "RootCnv/RootAddress.h"

#include <fstream>
#include <memory>

#include "MIHelpers.h"

namespace Gaudi {
  namespace Examples {
    namespace MultiInput {

      /// Write the content of the RootAddress of a data object
      class DumpAddress: public Algorithm {
      public:
        DumpAddress(const std::string &name, ISvcLocator *svcloc):
          Algorithm(name, svcloc), m_count(0) {
          declareProperty("OutputFile", m_output, "Name of the output file");
          declareProperty("ObjectPath", m_path, "Path to the object in the transient store");
        }
        StatusCode initialize() {
          StatusCode sc = Algorithm::initialize();
          if (sc.isFailure()) return sc;
          m_outputFile.open(m_output.c_str());
          m_count = 0;
          return sc;
        }

        StatusCode execute() {
          MsgStream log(msgSvc());
          SmartDataPtr<DataObject> obj(eventSvc(), m_path);
          if (obj) {
            Gaudi::RootAddress *addr = dynamic_cast<Gaudi::RootAddress*>(obj->registry()->address());
            if (addr) {
              m_outputFile << *addr << std::endl;
            } else {
              log << MSG::ERROR << "Event " << m_count << " does not have a Gaudi::RootAddress" << endmsg;
              return StatusCode::FAILURE;
            }
          } else {
            log << MSG::ERROR << "No data at " << m_path << endmsg;
            return StatusCode::FAILURE;
          }
          ++m_count;
          return StatusCode::SUCCESS;
        }

        StatusCode finalize() {
          m_outputFile.close();
          return Algorithm::finalize();
        }
      private:
        std::string m_output;
        std::string m_path;
        std::ofstream m_outputFile;
        long m_count;
      };
      DECLARE_COMPONENT(DumpAddress)
    }
  }
}

