#include "GaudiKernel/DataObjectHandleProperty.h"

#include "GaudiKernel/DataObjectHandleBase.h"
#include "GaudiKernel/Parsers.h"
#include "GaudiKernel/System.h"

#include <sstream>

namespace Gaudi
{
  namespace Parsers
  {
    StatusCode parse( DataObjectHandleBase& v, const std::string& s )
    {
      DataObjID id;
      auto sc = parse( id, s );
      if ( sc ) v.setKey( std::move( id ) );
      return sc;
    }
  }

  namespace Utils
  {
    std::ostream& toStream( const DataObjectHandleBase& v, std::ostream& o ) { return o << v; }
  }
}

//---------------------------------------------------------------------------

DataObjectHandleProperty::DataObjectHandleProperty( const std::string& name, DataObjectHandleBase& ref )
    : PropertyWithHandlers( name, typeid( DataObjectHandleBase ) ), m_pValue( &ref )
{
}

//---------------------------------------------------------------------------

StatusCode DataObjectHandleProperty::fromString( const std::string& s )
{
  if ( !Gaudi::Parsers::parse( *m_pValue, s ).isSuccess() ) {
    return StatusCode::FAILURE;
  }
  return useUpdateHandler() ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

//---------------------------------------------------------------------------

bool DataObjectHandleProperty::setValue( const DataObjectHandleBase& value )
{
  *m_pValue = value;
  return useUpdateHandler();
}

//---------------------------------------------------------------------------

std::string DataObjectHandleProperty::toString() const
{
  useReadHandler();
  return m_pValue->toString();
}

//---------------------------------------------------------------------------
void DataObjectHandleProperty::toStream( std::ostream& out ) const
{
  // implicitly invokes useReadHandler()
  out << toString();
}

//---------------------------------------------------------------------------

DataObjectHandleProperty& DataObjectHandleProperty::operator=( const DataObjectHandleBase& value )
{
  setValue( value );
  return *this;
}

//---------------------------------------------------------------------------

DataObjectHandleProperty* DataObjectHandleProperty::clone() const { return new DataObjectHandleProperty( *this ); }

//---------------------------------------------------------------------------

bool DataObjectHandleProperty::load( Property& destination ) const { return destination.assign( *this ); }

//---------------------------------------------------------------------------

bool DataObjectHandleProperty::assign( const Property& source ) { return fromString( source.toString() ).isSuccess(); }

//---------------------------------------------------------------------------

const DataObjectHandleBase& DataObjectHandleProperty::value() const
{
  useReadHandler();
  return *m_pValue;
}
