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
#ifndef GAUDIKERNEL_TYPENAMESTRING_H
#define GAUDIKERNEL_TYPENAMESTRING_H
#include <ostream>
#include <string>
#include <utility>

namespace Gaudi {
  namespace Utils {
    /// Helper class to parse a string of format "type/name"
    class TypeNameString {
      std::string m_type, m_name;
      bool        m_haveType;

    public:
      template <size_t N>
      TypeNameString( const char ( &tn )[N] ) : TypeNameString{std::string_view{tn}} {}
      TypeNameString( std::string_view sv ) : TypeNameString{std::string{sv}} {}
      TypeNameString( std::string tn ) : m_type{std::move( tn )} {
        const auto slash_pos = m_type.find_first_of( "/" );
        m_haveType           = slash_pos != std::string::npos;
        if ( m_haveType ) {
          m_name = m_type.substr( slash_pos + 1 );
          m_type = m_type.substr( 0, slash_pos );
        } else
          m_name = m_type;
      }

      TypeNameString( std::string tn, std::string_view deftyp ) : TypeNameString( std::move( tn ) ) {
        if ( !m_haveType ) m_type = deftyp;
      }

      // support passing a Gaudi::Property<std::string> without having to include Property.h
      // require that prop.value() can be used to construct a TypeNameString...
      template <class T, typename = std::enable_if_t<std::is_constructible<
                             TypeNameString, decltype( std::declval<const T&>().value() )>::value>>
      TypeNameString( const T& prop ) : TypeNameString( prop.value() ) {}

      const std::string& type() const { return m_type; }
      const std::string& name() const { return m_name; }
      bool               haveType() const { return m_haveType; }
    };

    /// Output stream operator for TypeNameString instances.
    inline std::ostream& operator<<( std::ostream& s, const TypeNameString& tn ) {
      return s << tn.type() << '/' << tn.name();
    }
  } // namespace Utils
} // namespace Gaudi

#endif // GAUDIKERNEL_TYPENAMESTRING_H
