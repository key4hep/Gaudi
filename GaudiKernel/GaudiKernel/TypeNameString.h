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
      TypeNameString( const char tn[] ) : TypeNameString( std::string{tn} ) {}
      TypeNameString( const std::string& tn ) : m_type{tn} {
        const auto slash_pos = m_type.find_first_of( "/" );
        m_haveType           = slash_pos != std::string::npos;
        if ( m_haveType ) {
          m_name = m_type.substr( slash_pos + 1 );
          m_type = m_type.substr( 0, slash_pos );
        } else
          m_name = m_type;
      }

      TypeNameString( const std::string& tn, const std::string& deftyp ) : TypeNameString( tn ) {
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
