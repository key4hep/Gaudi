/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiKernel/DataHandleProperty.h"

#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/System.h"
#include <Gaudi/Parsers/CommonParsers.h>

#include <sstream>

namespace Gaudi {
  namespace Parsers {
    StatusCode parse( DataHandle& v, std::string_view s ) {
      DataObjID id;
      auto      sc = parse( id, s );
      if ( sc ) v.setKey( std::move( id ) );
      return sc;
    }
  } // namespace Parsers

  namespace Utils {
    std::ostream& toStream( const Gaudi::DataHandle& v, std::ostream& o ) { return o << v; }
  } // namespace Utils
} // namespace Gaudi

//---------------------------------------------------------------------------

DataHandleProperty::DataHandleProperty( const std::string& name, Gaudi::DataHandle& ref )
    : PropertyWithHandlers( name, typeid( ref ) ), m_pValue( &ref ) {}

//---------------------------------------------------------------------------

StatusCode DataHandleProperty::fromString( const std::string& s ) {
  if ( !Gaudi::Parsers::parse( *m_pValue, s ).isSuccess() ) { return StatusCode::FAILURE; }
  return useUpdateHandler() ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

//---------------------------------------------------------------------------

bool DataHandleProperty::setValue( const Gaudi::DataHandle& value ) {
  *m_pValue = value;
  return useUpdateHandler();
}

//---------------------------------------------------------------------------

std::string DataHandleProperty::toString() const {
  useReadHandler();
  return m_pValue->objKey();
}

//---------------------------------------------------------------------------
void DataHandleProperty::toStream( std::ostream& out ) const {
  // implicitly invokes useReadHandler()
  out << toString();
}

//---------------------------------------------------------------------------

DataHandleProperty& DataHandleProperty::operator=( const Gaudi::DataHandle& value ) {
  setValue( value );
  return *this;
}

//---------------------------------------------------------------------------

DataHandleProperty* DataHandleProperty::clone() const { return new DataHandleProperty( *this ); }

//---------------------------------------------------------------------------

bool DataHandleProperty::load( Gaudi::Details::PropertyBase& destination ) const { return destination.assign( *this ); }

//---------------------------------------------------------------------------

bool DataHandleProperty::assign( const Gaudi::Details::PropertyBase& source ) {
  return fromString( source.toString() ).isSuccess();
}

//---------------------------------------------------------------------------

const Gaudi::DataHandle& DataHandleProperty::value() const {
  useReadHandler();
  return *m_pValue;
}
