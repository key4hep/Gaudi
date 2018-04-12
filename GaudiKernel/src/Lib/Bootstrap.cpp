#include <iostream>

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/System.h"

#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IClassManager.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ObjectFactory.h"
#include "GaudiKernel/Service.h"

#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IJobOptionsSvc.h"

#include "RVersion.h"

namespace Gaudi
{

  /** @class BootSvcLocator

      A dual-stage boostrap mechanism is used to ensure an orderly startup
      of the ApplicationMgr. If this function is called before the singleton
      ApplicationMgr instance exists, a BootstrapAppMgr singleton instance is
      created. This responds to any subsequent requests for services by
      returning StatusCode::FAILURE, unless the ApplicationMgr singleton
      instance has been created in the interim. In this case, the BootstrapAppMgr
      forwards the request to the ApplicationMgr instance. The motivation for
      this is to handle static object instances where the constructor attempts
      to locate services and would otherwise instantiate the ApplicationMgr
      instance in an unorderly manner. This logic requires that the
      ApplicationMgr instance is created explicitly.

  */
  class BootSvcLocator : public implements<ISvcLocator>
  {
  public:
#if !defined( GAUDI_V22_API ) || defined( G22_NEW_SVCLOCATOR )
    StatusCode getService( const Gaudi::Utils::TypeNameString& typeName, const InterfaceID& iid,
                           IInterface*& pinterface ) override;
    StatusCode getService( const Gaudi::Utils::TypeNameString& typeName, IService*& svc,
                           const bool createIf = true ) override;
#endif
    const std::list<IService*>& getServices() const override;
    bool existsService( const std::string& name ) const override;

    /// Returns a smart pointer to a service.
    SmartIF<IService>& service( const Gaudi::Utils::TypeNameString& typeName, const bool createIf = true ) override;
  };
}

static SmartIF<ISvcLocator> s_svclocInstance;
static SmartIF<IAppMgrUI>   s_appmgrInstance;

//------------------------------------------------------------------------------
IAppMgrUI* Gaudi::createApplicationMgr( const std::string& dllname, const std::string& factname )
//------------------------------------------------------------------------------
{
  // Allow not for multiple AppManagers: If already instantiated then just
  // return it
  if ( !s_appmgrInstance ) {
    s_appmgrInstance = createApplicationMgrEx( dllname, factname );
    s_svclocInstance = s_appmgrInstance;
  }
  return s_appmgrInstance.get();
}

//------------------------------------------------------------------------------
IAppMgrUI* Gaudi::createApplicationMgrEx( const std::string& dllname, const std::string& factname )
//------------------------------------------------------------------------------
{
  // Create an instance of the application Manager
  auto iif = make_SmartIF( Gaudi::createInstance( "ApplicationMgr", factname, dllname ) );
  // Locate few interfaces of the Application Manager
  return iif ? iif.as<IAppMgrUI>().get() : nullptr;
}

//------------------------------------------------------------------------------
ISvcLocator* Gaudi::svcLocator()
//------------------------------------------------------------------------------
//
// A dual-stage bootstrap mechanism is used to ensure an orderly startup
// of the ApplicationMgr. If this function is called before the singleton
// ApplicationMgr instance exists, a BootSvcLocator singleton instance is
// created. This responds to any subsequent requests for services by
// returning StatusCode::FAILURE, unless the ApplicationMgr singleton
// instance has been created in the interim. In this case, the BootSvcLocator
// forwards the request to the ApplicationMgr instance. The motivation for
// this is to handle static object instances where the constructor attempts
// to locate services and would otherwise instantiate the ApplicationMgr
// instance in an unorderly manner. This logic requires that the
// ApplicationMgr instance is created explicitly.

{
  if ( !s_svclocInstance ) {
    IAppMgrUI* iappmgr = createApplicationMgr();
    if ( iappmgr ) {
      s_svclocInstance = iappmgr;
      if ( s_svclocInstance ) return s_svclocInstance;
    }
    //---Reverted change to create a Minimal SvcLocator in case is requested before AppMgr is created
    // if( 0 == s_appmgrInstance ) {
    //  s_svclocInstance = new BootSvcLocator();
    //} else {
    //  StatusCode sc = s_appmgrInstance->queryInterface( ISvcLocator::interfaceID(),
    //                                                  pp_cast<void>(&s_svclocInstance) );
    //  if( sc.isSuccess() ) {
    //    return s_svclocInstance;
    //  }
    //}
  }
  return s_svclocInstance;
}

//------------------------------------------------------------------------------
ISvcLocator* Gaudi::setInstance( ISvcLocator* newInstance )
//------------------------------------------------------------------------------
{
  ISvcLocator* oldInstance = s_svclocInstance.get();
  s_svclocInstance         = newInstance;
  s_appmgrInstance         = s_svclocInstance;
  return oldInstance;
}

//------------------------------------------------------------------------------
IAppMgrUI* Gaudi::setInstance( IAppMgrUI* newInstance )
//------------------------------------------------------------------------------
{
  IAppMgrUI* oldInstance = s_appmgrInstance.get();
  s_appmgrInstance       = newInstance;
  s_svclocInstance       = s_appmgrInstance;
  return oldInstance;
}

//------------------------------------------------------------------------------
IInterface* Gaudi::createInstance( const std::string& name, const std::string& factname, const std::string& dllname )
//------------------------------------------------------------------------------
{

  IInterface* ii = ObjFactory::create( factname, nullptr );
  if ( ii ) return ii;
  IService* is = Service::Factory::create( factname, name, nullptr );
  if ( is ) return is;
  IAlgorithm* ia = Algorithm::Factory::create( factname, name, nullptr );
  if ( ia ) return ia;

  void* libHandle = nullptr;
  if ( System::loadDynamicLib( dllname, &libHandle ) ) {
    ii = ObjFactory::create( factname, nullptr );
    if ( ii ) return ii;
    is = Service::Factory::create( factname, name, nullptr );
    if ( is ) return is;
    ia = Algorithm::Factory::create( factname, name, nullptr );
    if ( ia ) return ia;
  } else {
    // DLL library not loaded. Try in the local module
    std::cout << System::getLastErrorString() << std::endl;
    std::cout << "Gaudi::Bootstrap: Not found DLL " << dllname << std::endl;
  }
  return nullptr;
}

//------------------------------------------------------------------------------
IAppMgrUI* Gaudi::createApplicationMgr( const std::string& dllname )
{
  //------------------------------------------------------------------------------
  return createApplicationMgr( dllname, "ApplicationMgr" );
}

//------------------------------------------------------------------------------
IAppMgrUI* Gaudi::createApplicationMgr()
{
  //------------------------------------------------------------------------------
  return createApplicationMgr( "GaudiCoreSvc", "ApplicationMgr" );
}

//=======================================================================
// BootSvcLocator
//=======================================================================

static std::list<IService*> s_bootServices;
static SmartIF<IService>    s_bootService;
static SmartIF<IInterface>  s_bootInterface;

using Gaudi::BootSvcLocator;

#if !defined( GAUDI_V22_API ) || defined( G22_NEW_SVCLOCATOR )
StatusCode Gaudi::BootSvcLocator::getService( const Gaudi::Utils::TypeNameString& typeName, const InterfaceID& iid,
                                              IInterface*& pinterface )
{
  StatusCode sc = StatusCode::FAILURE;
  if ( s_appmgrInstance ) {
    sc = s_svclocInstance->getService( typeName, iid, pinterface );
  } else {
    pinterface = s_bootInterface.get();
  }
  return sc;
}
StatusCode Gaudi::BootSvcLocator::getService( const Gaudi::Utils::TypeNameString& typeName, IService*& svc,
                                              const bool createIf )
{
  StatusCode sc = StatusCode::FAILURE;
  if ( s_appmgrInstance ) {
    sc = s_svclocInstance->getService( typeName, svc, createIf );
  } else {
    svc = s_bootService.get();
  }
  return sc;
}
#endif

const std::list<IService*>& Gaudi::BootSvcLocator::getServices() const
{
  return s_appmgrInstance ? s_svclocInstance->getServices() : s_bootServices;
}
bool Gaudi::BootSvcLocator::existsService( const std::string& name ) const
{
  return s_appmgrInstance && s_svclocInstance->existsService( name );
}

SmartIF<IService>& Gaudi::BootSvcLocator::service( const Gaudi::Utils::TypeNameString& typeName, const bool createIf )
{
  return s_appmgrInstance ? s_svclocInstance->service( typeName, createIf ) : s_bootService;
}

#ifdef GAUDI_HASCLASSVISIBILITY
#pragma GCC visibility push( default )
#endif
// Python bootstrap helpers
extern "C" {
#define PyHelper( x ) py_bootstrap_##x
IInterface* PyHelper( createApplicationMgr )() { return Gaudi::createApplicationMgr(); }
IInterface* PyHelper( getService )( IInterface* app, char* name )
{
  auto svcloc = SmartIF<ISvcLocator>( app );
  return svcloc ? svcloc->service<IInterface>( name ).get() : nullptr;
}
bool PyHelper( setProperty )( IInterface* p, char* name, char* value )
{
  auto prop = SmartIF<IProperty>( p );
  return prop && prop->setProperty( name, value ).isSuccess();
}
const char* PyHelper( getProperty )( IInterface* p, char* name )
{
  auto prop = SmartIF<IProperty>( p );
  return prop ? prop->getProperty( name ).toString().c_str() : nullptr;
}
bool PyHelper( configureApp )( IInterface* app )
{
  auto ui = SmartIF<IAppMgrUI>( app );
  return ui && ui->configure().isSuccess();
}
bool PyHelper( addPropertyToCatalogue )( IInterface* p, char* comp, char* name, char* value )
{
  auto jos = SmartIF<IJobOptionsSvc>( p );
  return jos && jos->addPropertyToCatalogue( comp, Gaudi::Property<std::string>( name, value ) ).isSuccess();
}
int PyHelper( ROOT_VERSION_CODE )() { return ROOT_VERSION_CODE; }

#define PyFSMHelper( s )                                                                                               \
  bool py_bootstrap_fsm_##s( IInterface* i )                                                                           \
  {                                                                                                                    \
    auto fsm = SmartIF<IStateful>( i );                                                                                \
    return fsm && fsm->s().isSuccess();                                                                                \
  }

PyFSMHelper( configure ) PyFSMHelper( initialize ) PyFSMHelper( start ) PyFSMHelper( stop ) PyFSMHelper( finalize )
    PyFSMHelper( terminate )

        bool py_bootstrap_app_run( IInterface* i, int maxevt )
{
  auto ep = SmartIF<IEventProcessor>( i );
  return ep && ep->executeRun( maxevt ).isSuccess();
}
}
#ifdef GAUDI_HASCLASSVISIBILITY
#pragma GCC visibility pop
#endif
