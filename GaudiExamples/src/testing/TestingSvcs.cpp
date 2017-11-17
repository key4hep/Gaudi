#include "GaudiKernel/Service.h"

namespace GaudiTesting
{

  /** Special service that issue a failure in one of the transitions (for testing).
   */
  class FailingSvc : public Service
  {
  public:
    /// Standard Constructor
    using Service::Service;

    StatusCode initialize() override
    {
      StatusCode sc = Service::initialize();
      if ( sc.isFailure() ) {
        error() << "failed to initialize base class" << endmsg;
        return sc;
      }
      return handle( "initialize" );
    }
    StatusCode start() override
    {
      StatusCode sc = Service::start();
      if ( sc.isFailure() ) {
        error() << "failed to start base class" << endmsg;
        return sc;
      }
      return handle( "start" );
    }
    StatusCode stop() override
    {
      StatusCode sc = handle( "stop" );
      if ( sc.isFailure() ) return sc;
      return Service::stop();
    }
    StatusCode finalize() override
    {
      StatusCode sc = handle( "finalize" );
      if ( sc.isFailure() ) return sc;
      return Service::finalize();
    }

  private:
    Gaudi::Property<std::string> m_transition{
        this, "Transition", "", "In which transition to fail ['initialize', 'start', 'stop',  'finalize']"};
    Gaudi::Property<std::string> m_mode{this, "Mode", "failure", "Type of failure ['failure',  'exception']"};

    inline StatusCode handle( const std::string& transition )
    {
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
}
