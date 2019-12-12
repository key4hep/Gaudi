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
#include "GaudiKernel/Service.h"

namespace GaudiTesting {

  /** Special service that issue a failure in one of the transitions (for testing).
   */
  class FailingSvc : public Service {
  public:
    /// Standard Constructor
    using Service::Service;

    StatusCode initialize() override {
      StatusCode sc = Service::initialize();
      if ( sc.isFailure() ) {
        error() << "failed to initialize base class" << endmsg;
        return sc;
      }
      return handle( "initialize" );
    }
    StatusCode start() override {
      StatusCode sc = Service::start();
      if ( sc.isFailure() ) {
        error() << "failed to start base class" << endmsg;
        return sc;
      }
      return handle( "start" );
    }
    StatusCode stop() override {
      StatusCode sc = handle( "stop" );
      if ( sc.isFailure() ) return sc;
      return Service::stop();
    }
    StatusCode finalize() override {
      StatusCode sc = handle( "finalize" );
      if ( sc.isFailure() ) return sc;
      return Service::finalize();
    }

  private:
    Gaudi::Property<std::string> m_transition{
        this, "Transition", "", "In which transition to fail ['initialize', 'start', 'stop',  'finalize']"};
    Gaudi::Property<std::string> m_mode{this, "Mode", "failure", "Type of failure ['failure',  'exception']"};

    inline StatusCode handle( const std::string& transition ) {
      if ( m_transition == transition ) {
        if ( m_mode == "exception" ) {
          throw GaudiException( "forced failure in " + transition, name(), StatusCode::FAILURE );
        } else if ( m_mode == "failure" ) {
          return StatusCode::FAILURE;
        } else {
          warning() << "Unknown type of failure '" << m_mode << "', use 'failure' or 'exception'" << endmsg;
        }
      }
      return StatusCode::SUCCESS;
    }
  };

  DECLARE_COMPONENT( FailingSvc )
} // namespace GaudiTesting
