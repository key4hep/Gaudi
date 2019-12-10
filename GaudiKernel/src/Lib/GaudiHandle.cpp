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
#include "GaudiKernel/GaudiHandle.h"
#include <iostream>
#include <string>

//
// GaudiHandleBase implementation
//

void GaudiHandleBase::setTypeAndName( std::string myTypeAndName ) { m_typeAndName = std::move( myTypeAndName ); }

std::string GaudiHandleBase::type() const {
  std::string::size_type slash = m_typeAndName.find( '/' );
  if ( slash != std::string::npos ) {
    // return only part before /
    return m_typeAndName.substr( 0, slash );
  } else {
    // return full string
    return m_typeAndName;
  }
}

std::string GaudiHandleBase::name() const {
  auto slash = m_typeAndName.find( '/' );
  if ( slash == std::string::npos ) {
    // only type is given, or string is empty.
    // return default name (=type or empty, in this case full string)
    return m_typeAndName;
  } else if ( slash != m_typeAndName.length() - 1 ) {
    // an explicit name is given, so return it
    return m_typeAndName.substr( slash + 1 );
  } else {
    // ends with /, i.e. explicit empty name.
    // Should probably never happen.
    return "";
  }
}

void GaudiHandleBase::setName( std::string_view myName ) { m_typeAndName = type().append( "/" ).append( myName); }

std::string GaudiHandleBase::pythonPropertyClassName() const { return componentType() + "Handle"; }

std::string GaudiHandleBase::messageName() const {
  std::string propName = propertyName();
  if ( propName.empty() ) { propName = pythonPropertyClassName() + "('" + m_typeAndName + "')"; }
  return parentName() + "." + propName;
}

std::string GaudiHandleBase::pythonRepr() const { return pythonPropertyClassName() + "('" + m_typeAndName + "')"; }

//
// GaudiHandleArrayBase implementation
//
bool GaudiHandleArrayBase::setTypesAndNames( const std::vector<std::string>& myTypesAndNamesList ) {
  clear();
  for ( const auto& it : myTypesAndNamesList ) {
    if ( !push_back( it ) ) return false;
  }
  return true;
}

const std::vector<std::string> GaudiHandleArrayBase::getBaseInfos( std::string ( GaudiHandleBase::*pMemFunc )()
                                                                       const ) const {
  std::vector<std::string> theList;
  for ( const auto& it : getBaseArray() ) theList.push_back( ( it->*pMemFunc )() );
  return theList;
}

const std::vector<std::string> GaudiHandleArrayBase::typesAndNames() const {
  return getBaseInfos( &GaudiHandleBase::typeAndName );
}

const std::vector<std::string> GaudiHandleArrayBase::types() const { return getBaseInfos( &GaudiHandleBase::type ); }

const std::vector<std::string> GaudiHandleArrayBase::names() const { return getBaseInfos( &GaudiHandleBase::name ); }

std::string GaudiHandleArrayBase::pythonPropertyClassName() const { return componentType() + "HandleArray"; }

std::string GaudiHandleArrayBase::pythonRepr() const {
  std::string repr    = pythonPropertyClassName() + "([";
  auto        theList = typesAndNames();
  auto        first   = theList.begin();
  auto        last    = theList.end();
  if ( first != last ) {
    repr += "'" + *first + "'";
    ++first;
  }
  for ( ; first != last; ++first ) repr += ",'" + *first + "'";
  repr += "])";
  return repr;
}

//
// Public functions
//
std::ostream& operator<<( std::ostream& os, const GaudiHandleInfo& handle ) {
  std::string msg;
  const auto& propName = handle.propertyName();
  if ( !propName.empty() ) msg += propName + " = ";
  msg += handle.pythonRepr();
  os << msg;
  return os;
}
