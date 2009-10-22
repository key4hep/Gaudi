#include "GaudiKernel/GaudiHandle.h"
#include <iostream>
#include <string>

//
// GaudiHandleBase implementation
//

void GaudiHandleBase::setTypeAndName( const std::string& myTypeAndName ) {
  m_typeAndName = myTypeAndName;
}

std::string GaudiHandleBase::type() const {
  std::string::size_type slash = m_typeAndName.find('/');
  if ( slash != std::string::npos ) {
    // return only part before /
    return m_typeAndName.substr(0,slash);
  } else {
    // return full string
    return m_typeAndName;
  }
}

std::string GaudiHandleBase::name() const {
  std::string::size_type slash = m_typeAndName.find('/');
  if ( slash == std::string::npos ) {
    // only type is given, or string is empty.
    // return default name (=type or empty, in this case full string)
    return m_typeAndName;
  } else if ( slash != m_typeAndName.length() -1 ) {
    // an explicit name is given, so return it
    return m_typeAndName.substr(slash+1);
  } else {
    // ends with /, i.e. explicit empty name.
    // Should probably never happen.
    return "";
  }
}

void GaudiHandleBase::setName( const std::string& myName ) {
  m_typeAndName = type() + '/' + myName;
}

const std::string GaudiHandleBase::pythonPropertyClassName() const {
  return componentType() + "Handle";
}

const std::string GaudiHandleBase::messageName() const {
  std::string propName = propertyName();
  if ( propName.empty() ) {
    propName = pythonPropertyClassName() + "('" + m_typeAndName + "')";
  }
  return parentName() + "." + propName;
}

const std::string GaudiHandleBase::pythonRepr() const {
  return pythonPropertyClassName() + "('" + m_typeAndName + "')";
}

//
// GaudiHandleArrayBase implementation
//
bool GaudiHandleArrayBase::setTypesAndNames(  const std::vector< std::string >& myTypesAndNamesList ) {
  clear();
  std::vector< std::string >::const_iterator it = myTypesAndNamesList.begin(), 
    itEnd = myTypesAndNamesList.end();
  for ( ; it != itEnd; ++it ) {
    if ( !push_back( *it ) ) return false;
  }
  return true;
}

const std::vector< std::string >
GaudiHandleArrayBase::getBaseInfos( std::string (GaudiHandleBase::*pMemFunc)() const ) const {
  std::vector< std::string > theList;
  const GaudiHandleArrayBase::ConstBaseHandleArray& baseArray = getBaseArray();
  GaudiHandleArrayBase::ConstBaseHandleArray::const_iterator it = baseArray.begin(),
    itEnd = baseArray.end();
  for ( ; it != itEnd; ++it ) theList.push_back( ((*it)->*pMemFunc)() );
  return theList;
}

const std::vector< std::string > GaudiHandleArrayBase::typesAndNames() const {
  return getBaseInfos( &GaudiHandleBase::typeAndName );
}

const std::vector< std::string > GaudiHandleArrayBase::types() const {
  return getBaseInfos( &GaudiHandleBase::type );
}
  
const std::vector< std::string > GaudiHandleArrayBase::names() const {
  return getBaseInfos( &GaudiHandleBase::name );
}

const std::string GaudiHandleArrayBase::pythonPropertyClassName() const {
  return componentType() + "HandleArray";
}

const std::string GaudiHandleArrayBase::pythonRepr() const {
  std::string repr = pythonPropertyClassName() + "([";
  const std::vector< std::string >& theList = typesAndNames();
  std::vector< std::string >::const_iterator it = theList.begin(),
    itEnd = theList.end(), itLast = itEnd - 1;
  for ( ; it != itEnd; ++it ) {
    repr += "'" + *it + "'";
    if ( it != itLast ) repr += ",";
  }
  repr += "])";
  return repr;
}


//
// Public functions
//
std::ostream& operator<<( std::ostream& os, const GaudiHandleInfo& handle ) {
  std::string msg;
  const std::string& propName = handle.propertyName();
  if ( !propName.empty() ) msg += propName + " = ";
  msg += handle.pythonRepr();
  os << msg;
  return os;
}

