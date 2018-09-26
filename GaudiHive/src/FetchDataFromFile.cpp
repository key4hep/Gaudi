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
#include "GaudiAlg/GaudiAlgorithm.h"

namespace Gaudi {
  namespace Hive {
    class FetchDataFromFile final : public ::Algorithm {
    public:
      FetchDataFromFile( const std::string& name, ISvcLocator* pSvcLocator ) : Algorithm( name, pSvcLocator ) {
        // make sure this algorithm is seen as reentrant by Gaudi
        this->setProperty( "Cardinality", 0 ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
      }
      StatusCode initialize() override {
        StatusCode sc = Algorithm::initialize();
        if ( sc ) {
          // this is a hack to reuse the automatic dependencies declaration
          for ( auto k : m_dataKeys ) { addDependency( k, Gaudi::DataHandle::Writer ); }
        }
        return sc;
      }
      StatusCode start() override {
        StatusCode sc = Algorithm::start();
        if ( sc ) {
          for ( const auto& k : outputDataObjs() ) {
            if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) ) debug() << "adding data key " << k << endmsg;
            evtSvc()->addPreLoadItem( k.key() ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
          }
        }
        return sc;
      }
      StatusCode execute() override { return evtSvc()->preLoad(); }

    private:
      Gaudi::Property<std::vector<std::string>> m_dataKeys{
          this, "DataKeys", {}, "list of objects to be read from file"};
    };
  } // namespace Hive
} // namespace Gaudi
using Gaudi::Hive::FetchDataFromFile;
DECLARE_COMPONENT( FetchDataFromFile )
