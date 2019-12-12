/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiKernel/DataObjectHandleProperty.h"

#include "GaudiKernel/DataObjectHandleBase.h"
#include "GaudiKernel/System.h"
#include <Gaudi/Parsers/CommonParsers.h>

#include <sstream>

namespace Gaudi {
  namespace Parsers {
    StatusCode parse( DataObjectHandleBase& v, const std::string& s ) {
      DataObjID id;
      auto      sc = parse( id, s );
      if ( sc ) v.setKey( std::move( id ) );
      return sc;
    }
  } // namespace Parsers

  namespace Utils {
    std::ostream& toStream( const DataObjectHandleBase& v, std::ostream& o ) { return o << v; }
  } // namespace Utils
} // namespace Gaudi

//---------------------------------------------------------------------------

DataObjectHandleProperty::DataObjectHandleProperty( const std::string& name, DataObjectHandleBase& ref )
    : PropertyWithHandlers( name, typeid( DataObjectHandleBase ) ), m_pValue( &ref ) {}

//---------------------------------------------------------------------------

StatusCode DataObjectHandleProperty::fromString( const std::string& s ) {
  if ( !Gaudi::Parsers::parse( *m_pValue, s ).isSuccess() ) { return StatusCode::FAILURE; }
  return useUpdateHandler() ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

//---------------------------------------------------------------------------

bool DataObjectHandleProperty::setValue( const DataObjectHandleBase& value ) {
  *m_pValue = value;
  return useUpdateHandler();
}

//---------------------------------------------------------------------------

std::string DataObjectHandleProperty::toString() const {
  useReadHandler();
  return m_pValue->toString();
}

//---------------------------------------------------------------------------
void DataObjectHandleProperty::toStream( std::ostream& out ) const {
  // implicitly invokes useReadHandler()
  out << toString();
}

//---------------------------------------------------------------------------

DataObjectHandleProperty& DataObjectHandleProperty::operator=( const DataObjectHandleBase& value ) {
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

const DataObjectHandleBase& DataObjectHandleProperty::value() const {
  useReadHandler();
  return *m_pValue;
}
