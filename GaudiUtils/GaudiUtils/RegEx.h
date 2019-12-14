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
#ifndef GAUDIUTILS_REGEX_H
#define GAUDIUTILS_REGEX_H
// ============================================================================
// Include files
// ============================================================================
#include <boost/regex.hpp>
#include <string>

/*
 *  Gaudi namespace declaration
 */
namespace Gaudi {
  namespace Utils {
    // ========================================================================
    /** RegeEx: nemspace to hold gaudi regular expression checking
     *
     *  @param matchOr: return true if test is in any of the regexps
     *  @param matchAnd: return true if test is in all of the regexps
     *
     *  @author Rob Lambert Rob.Lambert@cern.ch
     *  @date   2009-07-29
     */
    namespace RegEx {
      class matchList {
        std::vector<boost::regex> m_regs;

      public:
        template <typename C>
        matchList( const C& c ) {
          m_regs.reserve( c.size() );
          std::transform( std::begin( c ), std::end( c ), std::back_inserter( m_regs ),
                          []( typename C::const_reference i ) { return boost::regex{i}; } );
        }

        bool Or( const std::string& test ) const {
          return std::any_of( std::begin( m_regs ), std::end( m_regs ),
                              [&]( const boost::regex& r ) { return boost::regex_match( test, r ); } );
        }
        bool And( const std::string& test ) const {
          return std::all_of( std::begin( m_regs ), std::end( m_regs ),
                              [&]( const boost::regex& r ) { return boost::regex_match( test, r ); } );
        }
      };

      /** return true if the string is in any of the regex's
       *  @param  std::string test [IN]:  string to match
       *  @param  container<std::string> regexps  [IN]:  container of regex strings
       *         can be any container with a const_iterator, begin and end
       *
       *  If you need to do this more than once, please first create a matchList
       *  object, and then invoke its Or method.
       */
      template <typename T>
      bool matchOr( const std::string& test, const T& regexps ) {
        // compares the string in test, to the regexps in a container
        //
        return std::any_of( std::begin( regexps ), std::end( regexps ), [&]( typename T::const_reference i ) {
          return boost::regex_match( test, boost::regex{i} );
        } );
      }

      /** return true if the string is in all of the regex's
       *  @param  std::string test [IN]:  string to match
       *  @param  container<std::string> regexps  [IN]:  container of regex strings
       *        can be any container with a const_iterator, begin and end
       *
       *  If you need to do this more than once, please first create a matchList
       *  object, and then invoke its And method.
       */
      template <typename T>
      bool matchAnd( const std::string& test, const T& regexps ) {
        // compares the string in test, to the regexps in a container
        //
        return std::all_of( std::begin( regexps ), std::end( regexps ), [&]( typename T::const_reference i ) {
          return boost::regex_match( test, boost::regex{i} );
        } );
      }
    } // namespace RegEx
  }   // namespace Utils
} // namespace Gaudi
#endif
