/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Functional/Consumer.h>
#include <Gaudi/Interfaces/IParticlePropertySvc.h>

namespace GaudiTestSuite {
  /** Small algorithm using Gaudi::ParticlePropertySvc.
   *
   *  @author Marco CLEMENCIC
   *  @date   2008-05-23
   */
  struct GaudiPPS : public Gaudi::Functional::Consumer<void()> {
    using Consumer::Consumer;

    StatusCode initialize() override {
      return Consumer::initialize().andThen( [&] {
        using Gaudi::Interfaces::IParticlePropertySvc;
        if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Initialize" << endmsg;
        auto pps = service<IParticlePropertySvc>( "Gaudi::ParticlePropertySvc", true );
      } );
    }
    void operator()() const override {
      if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;
    }
  };

  DECLARE_COMPONENT( GaudiPPS )
} // namespace GaudiTestSuite
