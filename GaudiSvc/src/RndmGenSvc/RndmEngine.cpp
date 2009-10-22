//====================================================================
//	Random Engine implementation
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
#define GAUDI_RANDOMGENSVC_RndmEngine_CPP

// STL include files
#include <cfloat>

// Framework include files
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/MsgStream.h"
#include "RndmEngine.h"

/// Standard Service constructor
RndmEngine::RndmEngine(const std::string& name, ISvcLocator* loc)
: base_class(name, loc)
{
}

/// Standard Service destructor
RndmEngine::~RndmEngine()   {
}

/// Service override: initialization
StatusCode RndmEngine::initialize()   {
  StatusCode status = Service::initialize();
  if ( status.isSuccess() )   {
    status = setProperties();
    if ( status.isSuccess() )   {
      m_pIncidentSvc = serviceLocator()->service("IncidentSvc");
      if (!m_pIncidentSvc.isValid()) {
        status = StatusCode::FAILURE;
      }
    }
  }
  return status;
}

/// Service override: finalization
StatusCode RndmEngine::finalize()   {
  m_pIncidentSvc = 0; // release
  return Service::finalize();
}

/** IRndmEngine interface implementation  */
/// Input serialization from stream buffer. Restores the status of the generator engine.
StreamBuffer& RndmEngine::serialize(StreamBuffer& str)    {
  return str;
}

/// Output serialization to stream buffer. Saves the status of the generator engine.
StreamBuffer& RndmEngine::serialize(StreamBuffer& str) const    {
  return str;
}

/// Single shot returning single random number
double RndmEngine::rndm() const   {
  return DBL_MAX;
}

/// Single shot returning single random number
void RndmEngine::handle (const Incident& /* inc */ )    {
}

/** Multiple shots returning vector with flat random numbers.
    @param  array    Array containing random numbers
    @param  howmany  fill 'howmany' random numbers into array
    @param  start    ... starting at position start
    @return StatusCode indicating failure or success.
*/
StatusCode RndmEngine::rndmArray( std::vector<double>& array, long howmany, long start) const   {
  long cnt = start;
  array.resize(start+howmany);
  for ( long i = start, num = start+howmany; i < num; i++ )     {
    array[cnt++] = rndm();
  }
  return StatusCode::SUCCESS;
}

