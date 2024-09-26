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
#include <Gaudi/Examples/IKeyValueStore.h>
#include <GaudiKernel/Service.h>
#include <fmt/format.h>

namespace Gaudi::Examples::Cpp {
  class KeyValueStore : public extends<Service, IKeyValueStore> {
    using extends::extends;

    StatusCode initialize() override {
      return extends::initialize().andThen( [this] { info() << "Initialize " << name() << " (C++)" << endmsg; } );
    }
    std::optional<std::string> get( std::string_view key ) const override { return fmt::format( "{0}-{0}", key ); }
  };

  DECLARE_COMPONENT( KeyValueStore )
} // namespace Gaudi::Examples::Cpp
