#include "GaudiKernel/EventDataHandle.h"
#include "GaudiKernel/IDataHandleHolder.h"
#include <stdexcept>

namespace Gaudi
{
  const EventDataHandle::Metadata
    EventDataHandle::s_readHandleMetadata( AccessMode::Read ),
    EventDataHandle::s_writeHandleMetadata( AccessMode::Write );

  void EventDataHandle::registerToOwner( IDataHandleHolder& owner, AccessMode access )
  {
    switch ( access ) {
    case AccessMode::Read:
      owner.registerInput( *this );
      break;
    case AccessMode::Write:
      owner.registerOutput( *this );
      break;
    default:
      throw std::runtime_error( "Unsupported access mode" );
    }
  }

  void EventDataHandle::initialize( const IDataHandleHolder& owner )
  {
    auto whiteBoard = owner.eventSvc();
    if ( !whiteBoard ) throw std::runtime_error( "Failed to access whiteboard" );
    m_whiteBoard = whiteBoard.get();
  }
}
