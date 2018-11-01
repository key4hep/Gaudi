#include "GaudiKernel/EventDataHandle.h"
#include "GaudiKernel/IDataHandleHolder.h"
#include <stdexcept>

namespace Gaudi
{
  const EventDataHandle::Metadata EventDataHandle::s_readHandleMetadata( AccessMode::Read ),
      EventDataHandle::s_writeHandleMetadata( AccessMode::Write );

  void EventDataHandle::registerToOwner( IDataHandleHolder& owner ) { owner.registerDataHandle( *this ); }

  void EventDataHandle::initialize( const IDataHandleHolder& owner )
  {
    auto whiteBoard = owner.eventSvc();
    if ( !whiteBoard ) throw std::runtime_error( "Failed to access whiteboard" );
    m_whiteBoard = whiteBoard.get();
  }
}
