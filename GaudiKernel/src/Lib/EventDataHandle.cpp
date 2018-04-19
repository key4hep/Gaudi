#include "GaudiKernel/EventDataHandle.h"
#include "GaudiKernel/IDataHandleHolder.h"
#include <stdexcept>

namespace Gaudi
{
  namespace experimental
  {
    void EventDataHandle::registerToOwner( IDataHandleHolder& owner, AccessMode access )
    {
      switch ( access ) {
      case AccessMode::Read:
        owner.registerEventInput( *this );
        break;
      case AccessMode::Write:
        owner.registerEventOutput( *this );
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
}
