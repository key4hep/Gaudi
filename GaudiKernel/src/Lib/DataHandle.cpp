#include "GaudiKernel/DataHandle.h"

#include <sstream>
#include <map>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

using namespace Gaudi;

namespace Gaudi { 
  namespace Parsers {
    
    StatusCode 
    parse(DataHandle& v, const std::string& s)
    {
      // Just a placeholder: currently the properties only contain
      // the key

      std::string k(s);
      boost::erase_all(k,"'");

      v.updateKey( k );

      return StatusCode::SUCCESS;
    }

  } //> ns Parsers

  namespace Utils {

    std::ostream& 
    toStream(const DataHandle& v, std::ostream& o)
    {
      o << "'" << v.objKey() << "'";
      return o;
    }
    
  } //> ns Utils

} //> ns Gaudi

DataHandleProperty::DataHandleProperty( const std::string& name, 
                                      DataHandle& ref )
  : Property( name, typeid( DataHandle ) ), 
    m_pValue( &ref ) 
{}

DataHandleProperty::~DataHandleProperty()
{}

StatusCode 
DataHandleProperty::fromString(const std::string& s)
{
  if (!Gaudi::Parsers::parse(*m_pValue, s).isSuccess()) {
    return StatusCode::FAILURE;
  }
  return useUpdateHandler()
    ? StatusCode::SUCCESS
    : StatusCode::FAILURE;
}

bool
DataHandleProperty::setValue( const DataHandle& value ) 
{
  m_pValue->operator=(value);
  return useUpdateHandler();
}

std::string
DataHandleProperty::toString( ) const 
{
  useReadHandler();
  std::ostringstream o;
  Gaudi::Utils::toStream(*m_pValue, o);
  return o.str();
}

void
DataHandleProperty::toStream(std::ostream& out) const
{
  useReadHandler();
  out << this->toString();
}

