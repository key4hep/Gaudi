#include "GaudiKernel/DataHandleConfigurable.h"

#include "GaudiKernel/Parsers.h"


using DataHandleConfigurable = Gaudi::experimental::DataHandleConfigurable;

namespace Gaudi
{
  namespace Parsers
  {
    StatusCode parse( DataHandleConfigurable& v, const std::string& s )
    {
      // FIXME: Aiming for perfect compatibility with old DataObjID-based
      //        solution for now, customize the repr once this is working

      DataObjID id;
      StatusCode sc = parse( id, s );

      if(sc.isSuccess()) {
        v.setTargetKey(std::move(id));
      }
      return sc;
    }
  }

  namespace Utils
  {
    std::ostream& toStream( const DataHandleConfigurable& v, std::ostream& o )
    {
      // FIXME: Aiming for perfect compatibility with old DataObjID-based
      //        solution for now, customize the repr once this is working

      return toStream( v.targetKey(), o );
    }
  }
} //> ns Gaudi


std::ostream& operator<<(std::ostream& str, const DataHandleConfigurable& dhc)
{
  // FIXME: Aiming for perfect compatibility with old DataObjID-based solution
  //        for now, customize the repr once this is working
  str << dhc.targetKey();
  return str;
}
