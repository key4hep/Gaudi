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
#include <Gaudi/Application.h>
#include <GaudiKernel/AppReturnCode.h>
#include <GaudiKernel/Bootstrap.h>
#include <GaudiKernel/IAppMgrUI.h>
#include <GaudiKernel/IEventProcessor.h>
#include <GaudiKernel/IJobOptionsSvc.h>
#include <GaudiKernel/IMessageSvc.h>
#include <GaudiKernel/INamedInterface.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/Property.h>
#include <gsl/span>

#define GAUDI_ASSERT_THROW_NAME( cond, msg, name )                                                                     \
  if ( !cond ) throw GaudiException{msg, name, StatusCode::FAILURE};

#define GAUDI_ASSERT_THROW( cond, msg )                                                                                \
  if ( !cond ) throw GaudiException{msg, this->name(), StatusCode::FAILURE};

namespace {
  void consumeOptions( SmartIF<IProperty> prop, Gaudi::Application::Options& options ) {
    GAUDI_ASSERT_THROW_NAME( prop, "invalid IProperty", "Gaudi::Application" );
    std::string prefix      = SmartIF<INamedInterface>( prop )->name() + '.';
    const auto  prefix_size = prefix.size();

    auto opt = options.upper_bound( prefix );
    while ( opt != end( options ) && std::string_view( opt->first ).substr( 0, prefix_size ) == prefix ) {
      GAUDI_ASSERT_THROW_NAME( prop->setProperty( opt->first.substr( prefix_size ), opt->second ),
                               "failure setting " + opt->first + " to " + opt->second, "Gaudi::Application" );
      // drop processed option and increase iterator
      opt = options.erase( opt );
    }
  }
} // namespace

Gaudi::Application::Factory::ReturnType Gaudi::Application::create( std::string_view type, Options opts ) {
  if ( type == "Gaudi::Application" ) { return std::make_unique<Application>( std::move( opts ) ); }
  return Factory::create( type, std::move( opts ) );
}

Gaudi::Application::Application( Gaudi::Application::Options options ) {
  // # Prepare the application
  // - instantiate
  app = Gaudi::createApplicationMgr();
  GAUDI_ASSERT_THROW_NAME( app, "failure instantiating ApplicationMgr", "Gaudi::Application" );

  // - main configuration
  consumeOptions( app.as<IProperty>(), options );
  // - start minimal services
  GAUDI_ASSERT_THROW_NAME( app->configure(), "failure creating basic services", "Gaudi::Application" );

  consumeOptions( SmartIF<IMessageSvc>{app}.as<IProperty>(), options );

  // - prepare job configuration
  {
    auto sloc = app.as<ISvcLocator>();
    auto jos  = sloc->service<IJobOptionsSvc>( "JobOptionsSvc" );
    std::for_each( begin( options ), end( options ), [&jos]( const auto& item ) {
      std::string_view name    = item.first;
      const auto       sep_pos = name.find_last_of( '.' );
      std::string_view client  = name.substr( 0, sep_pos );
      name.remove_prefix( sep_pos + 1 );
      jos->addPropertyToCatalogue( std::string{client}, Gaudi::Property<std::string>{std::string{name}, item.second} )
          .ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
    } );
  }
}

Gaudi::Application::~Application() { app->terminate().ignore(); }

int Gaudi::Application::run() {
  auto prop      = app.as<IProperty>();
  auto processor = app.as<IEventProcessor>();

  Gaudi::Property<int> evtMax;
  evtMax.assign( prop->getProperty( "EvtMax" ) );

  // - get ready to process events
  if ( app->initialize() ) {
    if ( app->start() ) {
      // - main processing loop
      if ( !processor->executeRun( evtMax ) )
        setAppReturnCode( prop, Gaudi::ReturnCode::GenericFailure )
            .ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );

      app->stop().ignore();
    } else
      setAppReturnCode( prop, Gaudi::ReturnCode::GenericFailure )
          .ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
    app->finalize().ignore();
  } else
    setAppReturnCode( prop, Gaudi::ReturnCode::GenericFailure ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
  return getAppReturnCode( prop );
}

namespace {
  struct c_opt_t {
    char* key;
    char* value;
  };
} // namespace

#ifdef GAUDI_HASCLASSVISIBILITY
#  pragma GCC visibility push( default )
#endif

extern "C" {
// helper to invoke the factory from Python
Gaudi::Application* _py_Gaudi__Application__create( const char* name, const c_opt_t* options, long n ) {
  Gaudi::Application::Options opts;
  gsl::span                   py_opts{options, n};
  for ( auto& opt : py_opts ) { opts[opt.key] = opt.value; }
  return Gaudi::Application::create( name, std::move( opts ) ).release();
}
int  _py_Gaudi__Application__run( Gaudi::Application* self ) { return self->run(); }
void _py_Gaudi__Application__delete( Gaudi::Application* self ) { delete self; }
}

#ifdef GAUDI_HASCLASSVISIBILITY
#  pragma GCC visibility pop
#endif
