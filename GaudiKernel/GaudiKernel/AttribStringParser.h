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
#ifndef _ATTRIB_STRING_PARSER_H_
#define _ATTRIB_STRING_PARSER_H_

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/System.h"
#include <iterator>
#ifdef __clang__
#  pragma clang diagnostic push
// Hide warning message:
// boost/regex/v4/instances.hpp:128:17: warning: keyword is hidden by macro definition
#  pragma clang diagnostic ignored "-Wkeyword-macro"
#endif
#include <boost/regex.hpp>
#ifdef __clang__
#  pragma clang diagnostic pop
#endif

namespace Gaudi {
  namespace Utils {
    /** Parse attribute strings allowing iteration over the various attributes.
     * Example of usage:
     * \code{.cpp}
     * using Parser = Gaudi::Utils::AttribStringParser;
     * std::string input{"key1='value1' key2='value2'"};
     * for(auto attrib: Parser(input)) {
     *   std::cout << attrib.tag << " -> " << attrib.value << std::endl;
     * }
     * \endcode
     */
    class AttribStringParser {
    public:
      /// Simple class to wrap tag/value pairs.
      struct Attrib {
        std::string tag;
        std::string value;
      };

      /// Iterator to loop over the tag/value pairs in the attribute string.
      // This class is essentially a wrapper around boost::sregex_iterator.
      class Iterator : public std::iterator<std::input_iterator_tag, Attrib> {
      public:
        Iterator() = default;
        Iterator( const boost::sregex_iterator& it, bool expand_vars ) : m_it( it ), m_expandVars( expand_vars ) {
          // i_setAttrib();
        }
        Iterator( const Iterator& other ) : Iterator( other.m_it, other.m_expandVars ) {}
        Iterator( Iterator&& other ) : m_it( std::move( other.m_it ) ), m_expandVars( other.m_expandVars ) {}
        Iterator operator++( int ) {
          ++m_it;
          return *this;
        }
        Iterator operator++() {
          auto old = *this;
          ++m_it;
          return old;
        }
        reference operator*() {
          i_setAttrib();
          return m_attrib;
        }
        bool operator==( const Iterator& other ) { return m_it == other.m_it; }
        bool operator!=( const Iterator& other ) { return m_it != other.m_it; }

      private:
        /// Wrapped boost::sregex_iterator instance.
        boost::sregex_iterator m_it;
        bool                   m_expandVars = false;
        /// Cached Attrib instance.
        Attrib m_attrib;
        /// Helper method used to update the cached Attrib instance when
        /// dereferencing the iterator.
        void i_setAttrib() {
          static const boost::sregex_iterator endmark;
          if ( m_it != endmark ) {
            // if we have a match, we cache the values
            m_attrib = Attrib{( *m_it )[1], ( *m_it )[2]};
            if ( m_expandVars && m_attrib.value.find( "${" ) != std::string::npos ) {
              static const boost::regex varexp{"\\$\\{([^}]+)\\}"};
              auto                      i = 1;
              while ( i ) {
                i              = 0;
                m_attrib.value = boost::regex_replace(
                    m_attrib.value, varexp,
                    [&i]( const boost::smatch& m ) -> std::string {
                      const std::string name  = m[1];
                      const char*       cname = name.c_str();
                      if ( System::isEnvSet( cname ) ) {
                        ++i;
                        return System::getEnv( cname );
                      }
                      return m[0]; // do not expand unknown vars
                    },
                    boost::match_default | boost::format_all );
              }
            }
          } else {
            // otherwise we clean the cache
            m_attrib = Attrib();
          }
        }
      };

      /** Initialize the parsing of an attribute string.
       *
       * @param data: attribute string
       * @param expand_vars: if true (default) expand environment variables in values
       */
      AttribStringParser( std::string data, bool expand_vars = true ) : m_data( data ), m_expandVars( expand_vars ) {}

    private:
      std::string m_data;
      bool        m_expandVars;

      boost::sregex_iterator parse() const {
        static const boost::regex exp{"[[:space:]]*([^[:space:]]+)[[:space:]]*=[[:space:]]*'(.*?)'"};
        return boost::sregex_iterator( begin( m_data ), end( m_data ), exp );
      }
      friend Iterator begin( const AttribStringParser& );
    };
    inline AttribStringParser::Iterator begin( const AttribStringParser& parser ) {
      return AttribStringParser::Iterator( parser.parse(), parser.m_expandVars );
    }
    inline AttribStringParser::Iterator end( const AttribStringParser& /*parser*/ ) {
      return AttribStringParser::Iterator();
    }
  } // namespace Utils
} // namespace Gaudi

#endif
