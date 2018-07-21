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
#include "GaudiKernel/IRndmEngine.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/ObjectFactory.h"
#include "GaudiKernel/SmartIF.h"

#include "RndmGen.h"
#include "RndmGenSvc.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT( RndmGenSvc )

/// Service override: initialization
StatusCode RndmGenSvc::initialize()
{
  StatusCode status = Service::initialize();

  auto mgr = serviceLocator()->as<ISvcManager>();

  if ( status.isSuccess() ) {
    status = setProperties();
    if ( status.isSuccess() ) { // Check if the Engine service exists:
      // FIXME: (MCl) why RndmGenSvc cannot create the engine service in a standard way?
      const bool  CREATE   = false;
      std::string machName = name() + ".Engine";
      auto        engine   = serviceLocator()->service<IRndmEngine>( machName, CREATE );
      if ( !engine && mgr ) {
        using Gaudi::Utils::TypeNameString;
        engine = mgr->createService( TypeNameString( machName, m_engineName ) );
      }
      if ( engine ) {
        auto serial  = engine.as<ISerialize>();
        auto service = engine.as<IService>();
        if ( serial && service ) {
          status = service->sysInitialize();
          if ( status.isSuccess() ) {
            m_engine    = engine;
            m_serialize = serial;
            info() << "Using Random engine:" << m_engineName.value() << endmsg;
          }
        }
      }
    }
  }
  return status;
}

/// Service override: finalization
StatusCode RndmGenSvc::finalize()
{
  StatusCode status = Service::finalize();
  m_serialize.reset();
  if ( m_engine ) {
    m_engine.as<IService>()->finalize().ignore();
    m_engine.reset();
  }
  return status;
}

/** IRndmGenSvc interface implementation  */
/// Input serialization from stream buffer. Restores the status of the generator engine.
StreamBuffer& RndmGenSvc::serialize( StreamBuffer& str )
{
  if ( m_serialize ) return m_serialize->serialize( str );
  error() << "Cannot input serialize Generator settings!" << endmsg;
  return str;
}

/// Output serialization to stream buffer. Saves the status of the generator engine.
StreamBuffer& RndmGenSvc::serialize( StreamBuffer& str ) const
{
  if ( m_serialize ) return m_serialize->serialize( str );
  error() << "Cannot output serialize Generator settings!" << endmsg;
  return str;
}

/// Retrieve engine
IRndmEngine* RndmGenSvc::engine() { return m_engine.get(); }

/// Retrieve a valid generator from the service.
StatusCode RndmGenSvc::generator( const IRndmGen::Param& par, IRndmGen*& refpGen )
{
  auto pGen = SmartIF<IRndmGen>( ObjFactory::create( par.type(), m_engine.get() ).release() );
  if ( !pGen ) {
    refpGen = nullptr;
    return StatusCode::FAILURE;
  }
  refpGen = pGen.get();
  refpGen->addRef(); // insure the caller gets something with a refCount of (at least) one back...
  return refpGen->initialize( par );
}

// Single shot returning single random number
double RndmGenSvc::rndm() const { return m_engine ? m_engine->rndm() : -1; }

/*  Multiple shots returning vector with flat random numbers.
    @param  array    Array containing random numbers
    @param  howmany  fill 'howmany' random numbers into array
    @param  start    ... starting at position start
    @return StatusCode indicating failure or success.
*/
StatusCode RndmGenSvc::rndmArray( std::vector<double>& array, long howmany, long start ) const
{
  return m_engine ? m_engine->rndmArray( array, howmany, start ) : StatusCode::FAILURE;
}

// Allow to set new seeds
StatusCode RndmGenSvc::setSeeds( const std::vector<long>& seeds )
{
  return m_engine ? m_engine->setSeeds( seeds ) : StatusCode::FAILURE;
}

// Allow to get the seeds
StatusCode RndmGenSvc::seeds( std::vector<long>& seeds ) const
{
  return m_engine ? m_engine->seeds( seeds ) : StatusCode::FAILURE;
}
