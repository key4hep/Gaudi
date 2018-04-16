#ifndef GAUDIKERNEL_TYPENAMESTRING_H
#define GAUDIKERNEL_TYPENAMESTRING_H
#include "GaudiKernel/Property.h"
#include <ostream>
#include <string>

namespace Gaudi
{
  namespace Utils
  {
    /// Helper class to parse a string of format "type/name"
    class TypeNameString
    {
      std::string m_type, m_name;
      bool        m_haveType;

    public:
      TypeNameString( const char tn[] ) : TypeNameString( std::string{tn} ) {}
      TypeNameString( const std::string& tn ) : m_type{tn}
      {
        const auto slash_pos = m_type.find_first_of( "/" );
        m_haveType           = slash_pos != std::string::npos;
        if ( m_haveType ) {
          m_name = m_type.substr( slash_pos + 1 );
          m_type = m_type.substr( 0, slash_pos );
        } else
          m_name = m_type;
      }
      TypeNameString( const std::string& tn, const std::string& deftyp ) : TypeNameString( tn )
      {
        if ( !m_haveType ) {
          m_type = deftyp;
        }
      }
      template <class T, class V, class H>
      TypeNameString( const Gaudi::Property<T, V, H>& prop ) : TypeNameString( prop.value() )
      {
      }
      const std::string& type() const { return m_type; }
      const std::string& name() const { return m_name; }
      bool               haveType() const { return m_haveType; }
    };

    /// Output stream operator for TypeNameString instances.
    inline std::ostream& operator<<( std::ostream& s, const TypeNameString& tn )
    {
      return s << tn.type() << '/' << tn.name();
    }
  }
}

#endif // GAUDIKERNEL_TYPENAMESTRING_H
