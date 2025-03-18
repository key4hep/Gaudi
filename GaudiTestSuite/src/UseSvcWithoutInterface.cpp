/***********************************************************************************\
* (c) Copyright 2025 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Algorithm.h>
#include <Gaudi/Functional/Consumer.h>
#include <GaudiKernel/Service.h>

namespace Gaudi::TestSuite {
  class SvcWithoutInterface : public Service {
  public:
    using Service::Service;

    StatusCode initialize() override {
      return Service::initialize().andThen( [this] { info() << "initialized" << endmsg; } );
    }

    void doSomething() const { info() << "doing something" << endmsg; }
  };
  DECLARE_COMPONENT( SvcWithoutInterface )

  class UseSvcWithoutInterface : public Gaudi::Functional::Consumer<void()> {
  public:
    UseSvcWithoutInterface( const std::string& name, ISvcLocator* svcLoc )
        : Consumer( name, svcLoc ), m_svc( "Gaudi::TestSuite::SvcWithoutInterface/SvcWithoutInterface", name ) {}
    StatusCode initialize() override {
      return Algorithm::initialize()
          .andThen( [this] { info() << "initializing..." << endmsg; } )
          .andThen( [this] { m_svc->doSomething(); } )
          .andThen( [this] { info() << "initialized" << endmsg; } );
    }
    void operator()() const override {}

  private:
    // Add your member variables here
    ServiceHandle<SvcWithoutInterface> m_svc;
  };
  DECLARE_COMPONENT( UseSvcWithoutInterface )
} // namespace Gaudi::TestSuite
