/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Algorithm.h>
#include <Gaudi/Examples/IKeyValueStore.h>
#include <GaudiKernel/ServiceHandle.h>

namespace Gaudi::Examples {
  class KVSTestAlg : public Gaudi::Algorithm {
    using Algorithm::Algorithm;

    StatusCode execute( EventContext const& ) const override {
      info() << "entering KVSTestAlg::execute()" << endmsg;
      std::string_view key   = "abc";
      auto             value = m_kvstore->get( key );
      if ( !value ) {
        info() << key << " not present" << endmsg;
      } else {
        info() << key << " -> " << *value << endmsg;
      }
      info() << "leaving KVSTestAlg::execute()" << endmsg;
      return StatusCode::SUCCESS;
    }

    ServiceHandle<IKeyValueStore> m_kvstore{ this, "KVStore", "Gaudi::Examples::Cpp::KeyValueStore" };
  };

  DECLARE_COMPONENT( KVSTestAlg )
} // namespace Gaudi::Examples
