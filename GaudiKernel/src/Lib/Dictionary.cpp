/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/System.h>
namespace GaudiDict {
  std::string typeName( const std::type_info& typ );
  std::string vectorName( const std::type_info& typ );
  std::string vectorName( const std::string& typ );
  std::string pairName( const std::type_info& typ1, const std::type_info& typ2 );
  std::string relationName( const std::string& prefix, const std::type_info& typ1, const std::type_info& typ2 );
  std::string templateName1( std::string templ, const std::type_info& typ );
  std::string templateName1( std::string templ, const std::string& typ );
  std::string templateName2( std::string templ, const std::type_info& typ1, const std::type_info& typ2 );
  std::string templateName3( std::string templ, const std::type_info& typ1, const std::type_info& typ2,
                             const std::type_info& typ3 );
  std::string keyedContainerName( const std::string& prefix, const std::type_info& typ1 );
} // namespace GaudiDict

static std::string clean( std::string c ) {
  for ( size_t occ = c.find( " *" ); occ != std::string::npos; occ = c.find( " *" ) ) c.replace( occ, 2, "*" );
  return c;
}

std::string GaudiDict::typeName( const std::type_info& typ ) {
  std::string r = clean( System::typeinfoName( typ ) );
  // if ( r.compare(0,4,"enum") == 0 )  {
  //  r = "int";
  //}
  return r;
}

std::string GaudiDict::templateName1( std::string templ, const std::type_info& typ ) {
  return templateName1( std::move( templ ), typeName( typ ) );
}

std::string GaudiDict::templateName1( std::string s, const std::string& typ ) {
  s += "<";
  s += typ;
  s += ( s.back() == '>' ) ? " >" : ">"; // with C++11, we can have <somename<T>>...
  return clean( s );
}

std::string GaudiDict::templateName2( std::string s, const std::type_info& typ1, const std::type_info& typ2 ) {
  s += "<";
  s += typeName( typ1 );
  s += ",";
  s += typeName( typ2 );
  s += ( s.back() == '>' ) ? " >" : ">";
  return clean( s );
}

std::string GaudiDict::templateName3( std::string s, const std::type_info& typ1, const std::type_info& typ2,
                                      const std::type_info& typ3 ) {
  s += "<";
  s += typeName( typ1 );
  s += ",";
  s += typeName( typ2 );
  s += ",";
  s += typeName( typ3 );
  s += ( s.back() == '>' ) ? " >" : ">";
  return clean( s );
}

std::string GaudiDict::vectorName( const std::type_info& typ ) { return templateName1( "std::vector", typ ); }

std::string GaudiDict::vectorName( const std::string& typ ) { return templateName1( "std::vector", typ ); }

std::string GaudiDict::pairName( const std::type_info& typ1, const std::type_info& typ2 ) {
  return templateName2( "std::pair", typ1, typ2 );
}

std::string GaudiDict::keyedContainerName( const std::string& mgr_typ, const std::type_info& typ1 ) {
  std::string s = "KeyedContainer<";
  s += typeName( typ1 );
  s += ",";
  s += mgr_typ;
  if ( mgr_typ[mgr_typ.length() - 1] == '>' ) s += " ";
  s += ">";
  return clean( s );
}

std::string GaudiDict::relationName( const std::string& /* prefix */, const std::type_info& /* typ1 */,
                                     const std::type_info& /* typ2 */ ) {
  return "";
}
