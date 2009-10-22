//====================================================================
//	Random Generator service implementation
//--------------------------------------------------------------------
//
//	Package    : Gaudi/RndmGen ( The LHCb Offline System)
//	Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 29/10/99| Initial version                              | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#define GAUDI_RANDOMGENSVC_RNDMGENSVC_CPP

// STL include files
#include <cfloat>

// Framework include files
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/IRndmEngine.h"

#include "GaudiKernel/MsgStream.h"

#include "RndmGen.h"
#include "RndmGenSvc.h"

using ROOT::Reflex::PluginService;

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_SERVICE_FACTORY(RndmGenSvc)

/// Standard Service constructor
RndmGenSvc::RndmGenSvc(const std::string& nam, ISvcLocator* svc)
: base_class(nam, svc), m_engine(0), m_serialize(0)
{
  declareProperty("Engine", m_engineName = "HepRndm::Engine<CLHEP::RanluxEngine>");
}

/// Standard Service destructor
RndmGenSvc::~RndmGenSvc()   {
}

/// Service override: initialization
StatusCode RndmGenSvc::initialize()   {
  StatusCode status = Service::initialize();
  MsgStream log(msgSvc(), name());
  std::string machName = name()+".Engine";
  SmartIF<IRndmEngine> engine;
  SmartIF<ISvcManager> mgr(serviceLocator());

  if ( status.isSuccess() )   {
    status = setProperties();
    if ( status.isSuccess() )   {  // Check if the Engine service exists:
      // FIXME: (MCl) why RndmGenSvc cannot create the engine service in a standard way?
      const bool CREATE = false;
      engine = serviceLocator()->service(machName, CREATE);
      if ( !engine.isValid() && mgr.isValid() )   {
        using Gaudi::Utils::TypeNameString;
        engine = mgr->createService(TypeNameString(machName, m_engineName));
      }
      if ( engine.isValid() )   {
        SmartIF<ISerialize> serial(engine);
        SmartIF<IService>   service(engine);
        if ( serial.isValid( ) && service.isValid( ) )  {
          status = service->sysInitialize();
          if ( status.isSuccess() )   {
            m_engine = engine;
            m_serialize = serial;
            m_engine->addRef();
            m_serialize->addRef();
            log << MSG::INFO << "Using Random engine:" << m_engineName << endmsg;
            return status;
          }
        }
      }
    }
  }
  return status;
}

/// Service override: finalization
StatusCode RndmGenSvc::finalize()   {
  StatusCode status = Service::finalize();
  if ( m_serialize ) m_serialize->release();
  m_serialize = 0;
  if ( m_engine ) {
    SmartIF<IService> service(m_engine);
    service->finalize().ignore();
    m_engine->release();
  }
  m_engine = 0;
  return status;
}

/** IRndmGenSvc interface implementation  */
/// Input serialization from stream buffer. Restores the status of the generator engine.
StreamBuffer& RndmGenSvc::serialize(StreamBuffer& str)    {
  if ( 0 != m_serialize )    {
    return m_serialize->serialize(str);
  }
  MsgStream log(msgSvc(), name());
  log << MSG::ERROR << "Cannot input serialize Generator settings!" << endmsg;
  return str;
}

/// Output serialization to stream buffer. Saves the status of the generator engine.
StreamBuffer& RndmGenSvc::serialize(StreamBuffer& str) const    {
  if ( 0 != m_serialize )    {
    return m_serialize->serialize(str);
  }
  MsgStream log(msgSvc(), name());
  log << MSG::ERROR << "Cannot output serialize Generator settings!" << endmsg;
  return str;
}

/// Retrieve engine
IRndmEngine* RndmGenSvc::engine()     {
  return m_engine;
}

/// Retrieve a valid generator from the service.
StatusCode RndmGenSvc::generator(const IRndmGen::Param& par, IRndmGen*& refpGen)   {
  StatusCode status = StatusCode::FAILURE;
  IInterface* iface = PluginService::CreateWithId<IInterface*>(par.type(),(IInterface*)m_engine);
  if ( iface ) {
    // query requested interface (adds ref count)
    status = iface->queryInterface(IRndmGen::interfaceID(), (void**)& refpGen);
    if ( status.isSuccess() )   {
      status = refpGen->initialize(par);
    }
    else  {
      iface->release();
    }
  }
  // Error!
  return status;
}

// Single shot returning single random number
double RndmGenSvc::rndm() const   {
  if ( 0 != m_engine )    {
    return m_engine->rndm();
  }
  return -1;
}

/*  Multiple shots returning vector with flat random numbers.
    @param  array    Array containing random numbers
    @param  howmany  fill 'howmany' random numbers into array
    @param  start    ... starting at position start
    @return StatusCode indicating failure or success.
*/
StatusCode RndmGenSvc::rndmArray( std::vector<double>& array, long howmany, long start) const   {
  if ( 0 != m_engine )    {
    return m_engine->rndmArray(array, howmany, start);
  }
  return StatusCode::FAILURE;
}

// Allow to set new seeds
StatusCode RndmGenSvc::setSeeds(const std::vector<long>& seeds)   {
  if ( 0 != m_engine )    {
    return m_engine->setSeeds(seeds);
  }
  return StatusCode::FAILURE;
}

// Allow to get the seeds
StatusCode RndmGenSvc::seeds(std::vector<long>& seeds)  const  {
  if ( 0 != m_engine )    {
    return m_engine->seeds(seeds);
  }
  return StatusCode::FAILURE;
}

