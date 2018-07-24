#include "GaudiKernel/DataHandleConfigurable.h"

#include "GaudiKernel/Parsers.h"

using DataHandleConfigurable = Gaudi::DataHandleConfigurable;

namespace Gaudi
{
  namespace Parsers
  {
    StatusCode parse( DataHandleConfigurable& v, const std::string& s )
    {
      DataObjID  id;
      StatusCode sc = parse( id, s );

      if ( sc.isSuccess() ) {
        v.setTargetKey( std::move( id ) );
      }
      return sc;
    }
  }

  namespace Utils
  {
    std::ostream& toStream( const DataHandleConfigurable& v, std::ostream& o )
    {
      o << "DataHandle(path=";
      toStream( v.targetKey(), o );
      o << ", whiteboard='" << v.metadata().whiteBoard() << '\'';
      o << ", access=DataHandle.AccessMode.";
      switch ( v.metadata().access() ) {
        case Gaudi::IDataHandleMetadata::AccessMode::Read:
          o << "Read";
          break;
        case Gaudi::IDataHandleMetadata::AccessMode::Write:
          o << "Write";
          break;
      }
      o << ')';
      return o;
    }
  }
} //> ns Gaudi

std::ostream& operator<<( std::ostream& str, const DataHandleConfigurable& dhc )
{
  return Gaudi::Utils::toStream( dhc, str );
}
