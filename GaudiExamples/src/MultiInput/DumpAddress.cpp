/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiKernel/Algorithm.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "RootCnv/RootAddress.h"

#include <fstream>
#include <memory>

#include "MIHelpers.h"

namespace Gaudi {
  namespace Examples {
    namespace MultiInput {

      /// Write the content of the RootAddress of a data object
      class DumpAddress : public ::Algorithm {
      public:
        using Algorithm::Algorithm;

        StatusCode initialize() override {
          StatusCode sc = Algorithm::initialize();
          if ( sc.isFailure() ) return sc;
          m_outputFile.open( m_output.value().c_str() );
          m_count = 0;
          return sc;
        }

        StatusCode execute() override {
          MsgStream                log( msgSvc() );
          SmartDataPtr<DataObject> obj( eventSvc(), m_path );
          if ( obj ) {
            Gaudi::RootAddress* addr = dynamic_cast<Gaudi::RootAddress*>( obj->registry()->address() );
            if ( addr ) {
              m_outputFile << *addr << std::endl;
            } else {
              log << MSG::ERROR << "Event " << m_count << " does not have a Gaudi::RootAddress" << endmsg;
              return StatusCode::FAILURE;
            }
          } else {
            log << MSG::ERROR << "No data at " << m_path.value() << endmsg;
            return StatusCode::FAILURE;
          }
          ++m_count;
          return StatusCode::SUCCESS;
        }

        StatusCode finalize() override {
          m_outputFile.close();
          return Algorithm::finalize();
        }

      private:
        Gaudi::Property<std::string> m_output{ this, "OutputFile", {}, "Name of the output file" };
        Gaudi::Property<std::string> m_path{ this, "ObjectPath", {}, "Path to the object in the transient store" };
        std::ofstream                m_outputFile;
        long                         m_count = 0;
      };
      DECLARE_COMPONENT( DumpAddress )
    } // namespace MultiInput
  }   // namespace Examples
} // namespace Gaudi
