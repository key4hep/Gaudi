#ifndef GAUDIUTILS_REGEX_H
#define GAUDIUTILS_REGEX_H
// ============================================================================
// Include files
// ============================================================================
#include <string>
#include <boost/regex.hpp>

/*
 *  Gaudi namespace declaration
 */
namespace Gaudi
{
  namespace  Utils
  {
    // ========================================================================
    /** RegeEx: nemspace to hold gaudi regular expression checking
     *
     *  @param matchOr: return true if test is in any of the regexps
     *  @param matchAnd: return true if test is in all of the regexps
     *
     *  @author Rob Lambert Rob.Lambert@cern.ch
     *  @date   2009-07-29
     */
    namespace RegEx
    {
      /** return true if the string is in any of the regex's
       *  @param  std::string test [IN]:  string to match
       *  @param  container<std::string> regexps  [IN]:  container of regex strings
       *         can be any container with a const_iterator, begin and end
       */
      template <typename T> bool matchOr(const std::string & test, const T & regexps)
      {
        //compares the string in test, to the regexps in a container
        //
        //@TODO: would be better the container contaied boost::regex
        //       instead of strings that need to be turned in boost::regex
        //       again and again...
        return std::any_of( std::begin(regexps), std::end(regexps),
                            [&](typename T::const_reference i) {
                return  boost::regex_match(test, boost::regex{i});
        });
      }

      /** return true if the string is in all of the regex's
       *  @param  std::string test [IN]:  string to match
       *  @param  container<std::string> regexps  [IN]:  container of regex strings
       *        can be any container with a const_iterator, begin and end
       */
      template <typename T> bool matchAnd(const std::string & test, const T & regexps)
      {
        //compares the string in test, to the regexps in a container
        //
        //@TODO: would be better the container contaied boost::regex
        //       instead of strings that need to be turned in boost::regex
        //       again and again...
        return std::all_of( std::begin(regexps), std::end(regexps),
                            [&](typename T::const_reference i) {
                return  boost::regex_match(test, boost::regex{i});
        });
      }
    }
  }
}
#endif
