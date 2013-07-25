#include <iostream>

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/System.h"

#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IClassManager.h"

#include "GaudiKernel/ObjectFactory.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/Algorithm.h"

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
  class BootSvcLocator : public implements1<ISvcLocator> {
  public:
    BootSvcLocator();
    virtual ~BootSvcLocator();
#if !defined(GAUDI_V22_API)|| defined(G22_NEW_SVCLOCATOR)
    virtual StatusCode getService( const Gaudi::Utils::TypeNameString& typeName,
                                   const InterfaceID& iid,
                                   IInterface*& pinterface );
    virtual StatusCode getService( const Gaudi::Utils::TypeNameString& typeName,
                                   IService*& svc,
                                   const bool createIf = true);
#endif
    virtual const std::list<IService*>& getServices( ) const;
    virtual bool existsService( const std::string& name ) const;

    /// Returns a smart pointer to a service.
    virtual SmartIF<IService> &service(const Gaudi::Utils::TypeNameString &typeName, const bool createIf = true);

  };
}


static SmartIF<ISvcLocator> s_svclocInstance;
static SmartIF<IAppMgrUI>   s_appmgrInstance;

//------------------------------------------------------------------------------
IAppMgrUI* Gaudi::createApplicationMgr(const std::string& dllname,
                                       const std::string& factname)
//------------------------------------------------------------------------------
{
  // Allow not for multiple AppManagers: If already instantiated then just
  // return it
  if ( !s_appmgrInstance.isValid() )    {
    s_appmgrInstance = createApplicationMgrEx(dllname, factname);
    s_svclocInstance = s_appmgrInstance;
  }
  return s_appmgrInstance.get();
}

//------------------------------------------------------------------------------
IAppMgrUI* Gaudi::createApplicationMgrEx(const std::string& dllname,
                                         const std::string& factname)
//------------------------------------------------------------------------------
{
  StatusCode     status;
  IInterface*    iif;
  IAppMgrUI*     iappmgr;

  // Create an instance of the application Manager
  iif = Gaudi::createInstance( "ApplicationMgr", factname, dllname );
  if( iif == 0 ) {
    return 0;
  }
  // Locate few interfaces of the Application Manager
  status = iif->queryInterface( IAppMgrUI::interfaceID(), pp_cast<void>(&iappmgr) );
  if( status.isFailure() ) {
    return 0;
  }
  iif->release();
  return iappmgr;
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
  if( !s_svclocInstance.isValid() )   {
    IAppMgrUI* iappmgr = createApplicationMgr();
    if( iappmgr ) {
      s_svclocInstance = iappmgr;
      if( s_svclocInstance.isValid() ) {
        return s_svclocInstance;
      }
    }
    //---Reverted change to create a Minimal SvcLocator in case is requested before AppMgr is created
    //if( 0 == s_appmgrInstance ) {
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
ISvcLocator* Gaudi::setInstance(ISvcLocator* newInstance)
//------------------------------------------------------------------------------
{
  ISvcLocator* oldInstance = s_svclocInstance.get();
  s_svclocInstance = newInstance;
  s_appmgrInstance = s_svclocInstance;
  return oldInstance;
}

//------------------------------------------------------------------------------
IAppMgrUI* Gaudi::setInstance(IAppMgrUI* newInstance)
//------------------------------------------------------------------------------
{
  IAppMgrUI* oldInstance = s_appmgrInstance.get();
  s_appmgrInstance = newInstance;
  s_svclocInstance = s_appmgrInstance;
  return oldInstance;
}

//------------------------------------------------------------------------------
IInterface* Gaudi::createInstance( const std::string& name,
                                                const std::string& factname,
                                                const std::string& dllname)
//------------------------------------------------------------------------------
{

  IInterface* ii = ObjFactory::create(factname, (IInterface*)0);
  if ( ii ) return ii;
  IService* is = Service::Factory::create(factname, name, (ISvcLocator*)0);
  if ( is ) return is;
  IAlgorithm* ia = Algorithm::Factory::create(factname, name, (ISvcLocator*)0);
  if ( ia ) return ia;

  StatusCode status;
  void* libHandle = 0;
  status = System::loadDynamicLib( dllname, &libHandle);
  if ( status.isSuccess() )   {
    ii = ObjFactory::create(factname, (IInterface*)0);
    if ( ii ) return ii;
    is = Service::Factory::create(factname, name, (ISvcLocator*)0);
    if ( is ) return is;
    ia = Algorithm::Factory::create(factname, name, (ISvcLocator*)0);
    if ( ia ) return ia;

    return 0;
  }
  else {
    // DLL library not loaded. Try in the local module
    std::cout << System::getLastErrorString() << std::endl;
    std::cout << "Gaudi::Bootstrap: Not found DLL " << dllname << std::endl;
    return 0;
  }
}

namespace {
  class ShadowEntry    {
  public:
    std::string dllName;
    std::string facName;
    IFactory*   fac;
  public:
    ShadowEntry() {
    }
    ShadowEntry(const std::string& d, const std::string& n, const IFactory* f) {
      dllName = d;
      facName = n;
      fac     = const_cast<IFactory*>(f);
    }
    ShadowEntry(const ShadowEntry& copy)   {
      dllName = copy.dllName;
      facName = copy.facName;
      fac     = copy.fac;
    }
  };
}

//------------------------------------------------------------------------------
IAppMgrUI* Gaudi::createApplicationMgr(const std::string& dllname )    {
//------------------------------------------------------------------------------
  return createApplicationMgr(dllname, "ApplicationMgr");
}

//------------------------------------------------------------------------------
IAppMgrUI* Gaudi::createApplicationMgr()    {
//------------------------------------------------------------------------------
  return createApplicationMgr("GaudiCoreSvc", "ApplicationMgr");
}

//=======================================================================
// BootSvcLocator
//=======================================================================

static std::list<IService*> s_bootServices;
static SmartIF<IService>    s_bootService;
static SmartIF<IInterface>  s_bootInterface;

using Gaudi::BootSvcLocator;

BootSvcLocator::BootSvcLocator() {
}
BootSvcLocator::~BootSvcLocator() {
}

#if !defined(GAUDI_V22_API) || defined(G22_NEW_SVCLOCATOR)
StatusCode Gaudi::BootSvcLocator::getService( const Gaudi::Utils::TypeNameString& typeName,
                                       const InterfaceID& iid,
                                       IInterface*& pinterface ) {
  StatusCode sc = StatusCode::FAILURE;
  if ( s_appmgrInstance.isValid() ) {
    sc = s_svclocInstance->getService(typeName, iid, pinterface );
  } else {
    pinterface = s_bootInterface.get();
  }
  return sc;
}
StatusCode Gaudi::BootSvcLocator::getService( const Gaudi::Utils::TypeNameString& typeName,
                                       IService*& svc,
                                       const bool createIf ) {
  StatusCode sc = StatusCode::FAILURE;
  if ( s_appmgrInstance.isValid() ) {
    sc = s_svclocInstance->getService(typeName, svc, createIf );
  } else {
    svc = s_bootService.get();
  }
  return sc;
}
#endif

const std::list<IService*>& Gaudi::BootSvcLocator::getServices( ) const {
  StatusCode sc = StatusCode::FAILURE;
  if ( s_appmgrInstance.isValid() ) {
    return s_svclocInstance->getServices( );
  } else {
    return s_bootServices;
  }
}
bool Gaudi::BootSvcLocator::existsService( const std::string& name ) const {
  bool result = false;
  if ( s_appmgrInstance.isValid() ) {
    result = s_svclocInstance->existsService(name);
  }
  return result;
}


SmartIF<IService>& Gaudi::BootSvcLocator::service(const Gaudi::Utils::TypeNameString &typeName, const bool createIf) {
  if ( s_appmgrInstance.isValid() ) {
    return s_svclocInstance->service(typeName, createIf);
  } else {
    return s_bootService;
  }
}
