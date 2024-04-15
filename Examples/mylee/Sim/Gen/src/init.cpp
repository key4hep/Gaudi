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
#include <Gaudi/Functional/Transformer.h>
#include <Gaudi/Property.h>
#include <GaudiKernel/EventContext.h>
#include <fmt/format.h>
#include <mylee/mc/gen_header.h>
#include <string>

namespace mylee::mc::gen {
  struct init : Gaudi::Functional::Transformer<gen_header( const EventContext& )> {
    init( const std::string& name, ISvcLocator* svcLoc )
        : Transformer{ name, svcLoc, KeyValue{ "header", "gen/header" } } {}

    StatusCode initialize() override {
      return Transformer::initialize().andThen( [this]() {
        if ( m_base_seed == 0 ) {
          error() << fmt::format( "the property {}.{} must be explicitly set", name(), m_base_seed.name() ) << endmsg;
          return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
      } );
    }

    gen_header operator()( const EventContext& ctx ) const override {
      return { { m_base_seed.value(), static_cast<long>( ctx.evt() ) }, m_generator.value() };
    }

    Gaudi::Property<std::string> m_generator{ this, "Generator", {}, "Name of the generator used for the event" };
    Gaudi::Property<long>        m_base_seed{ this, "BaseSeed", 0,
                                       "base seed for all events, to be set to a different value for each job" };
  };
  DECLARE_COMPONENT( init )
} // namespace mylee::mc::gen
