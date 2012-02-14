#ifndef GAUDIKERNEL_TYPENAMESTRING_H
#define GAUDIKERNEL_TYPENAMESTRING_H
#include <string>

namespace Gaudi {
  namespace Utils {
    /// Helper class to parse a string of format "type/name"
    class TypeNameString {
      std::string m_type, m_name;
      bool m_haveType;
      void init(const std::string& tn, const std::string& deftyp)    {
        const std::string::size_type slash_pos = tn.find_first_of("/");
        m_haveType = slash_pos != std::string::npos;
        m_name = (m_haveType) ? tn.substr( slash_pos + 1) : tn;
        m_type = (m_haveType) ? tn.substr( 0, slash_pos ) : deftyp;
      }
    public:
      TypeNameString(const char tn[]): m_haveType(false) { init(tn, tn); }
      TypeNameString(const std::string& tn): m_haveType(false) { init(tn, tn); }
      TypeNameString(const std::string& tn, const std::string& deftyp) { init(tn, deftyp); m_haveType = true; }
      const std::string& type() const { return m_type; }
      const std::string& name() const { return m_name; }
      bool haveType() const { return m_haveType; }
    };

    /// Output stream operator for TypeNameString instances.
    inline std::ostream& operator<< (std::ostream& s, const TypeNameString& tn) {
      return s << tn.type() << '/' << tn.name();
    }
  }
}


#endif // GAUDIKERNEL_TYPENAMESTRING_H
