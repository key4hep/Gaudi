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
#pragma once
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

// ============================================================================
#include <GaudiKernel/HistoDef.h>
#include <GaudiKernel/Map.h>
#include <GaudiKernel/StatusCode.h>
// ============================================================================
#define PARSERS_DECL_FOR_SINGLE( Type ) GAUDI_API StatusCode parse( Type& result, std::string_view input );

#define PARSERS_DECL_FOR_PAIR( FirstType, SecondType )                                                                 \
  GAUDI_API StatusCode parse( std::pair<FirstType, SecondType>& result, std::string_view input );

#define PARSERS_DECL_FOR_LIST( InnerType )                                                                             \
  GAUDI_API StatusCode parse( std::vector<InnerType>& result, std::string_view input );

#define PARSERS_DECL_FOR_SET( InnerType )                                                                              \
  GAUDI_API StatusCode parse( std::set<InnerType>& result, std::string_view input );                                   \
  GAUDI_API StatusCode parse( std::unordered_set<InnerType>& result, std::string_view input );

// ============================================================================
/** @file
 *  The declaration of major parsing functions used e.g
 *  for (re)implementation of new extended properties see class Property
 *  These function also could be used in a different, much wider contex.
 *  all of them have the semantic:
 *  <c>StatusCode parse ( TYPE& result , std::string_view input )</c>
 *  where <c>input</c> is the input string to be parsed,
 *  and <c>result</c> is the the result of parsing
 *
 *  @code
 *
 *  const std::string        input = ... ;
 *  std::vector<std::string> result ;
 *
 *  // parse the input
 *  StatusCode sc = parse ( result , input ) ;
 *  if ( sc.isFailure() )
 *  {
 *    // error here ...
 *  }
 *  std::cout << "vector size " << result.size() << std::endl ;
 *
 *  @endcode
 *
 *  @see Gaudi::Parsers::parse
 *  @see Property
 *
 *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
 *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
 *  @date 2006-05-12
 */
// ============================================================================
namespace Gaudi {
  // ==========================================================================
  class Histo1DDef;
  // ==========================================================================
  namespace Parsers {
    // ========================================================================
    /** parse the <c>bool</c> value
     *  @see Gaudi::Parsers::BoolGrammar
     *  @param result (output) boolean result
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    PARSERS_DECL_FOR_SINGLE( bool )
    // ========================================================================
    /** parse the <c>char</c> value
     *
     *  @see Gaudi::Parsers::CharGrammar
     *  @param result (output) boolean result
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    PARSERS_DECL_FOR_SINGLE( char )
    /// @see Gaudi::Parsers::parser( char&, const std::string& )
    PARSERS_DECL_FOR_SINGLE( unsigned char )
    /// @see Gaudi::Parsers::parser( char&, const std::string& )
    PARSERS_DECL_FOR_SINGLE( signed char )
    // ========================================================================
    /** parse the <c>int</c> value
     *
     *  @see Gaudi::Parsers::IntGrammar
     *  @param result (output) integer result
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    PARSERS_DECL_FOR_SINGLE( int )
    /// @see Gaudi::Parsers::parser( int&, const std::string& )
    PARSERS_DECL_FOR_SINGLE( short )
    /// @see Gaudi::Parsers::parser( int&, const std::string& )
    PARSERS_DECL_FOR_SINGLE( unsigned short )
    /// @see Gaudi::Parsers::parser( int&, const std::string& )
    PARSERS_DECL_FOR_SINGLE( unsigned int )
    /// @see Gaudi::Parsers::parser( int&, const std::string& )
    PARSERS_DECL_FOR_SINGLE( long )
    /// @see Gaudi::Parsers::parser( int&, const std::string& )
    PARSERS_DECL_FOR_SINGLE( unsigned long )
    /// @see Gaudi::Parsers::parser( int&, const std::string& )
    PARSERS_DECL_FOR_SINGLE( long long )
    /// @see Gaudi::Parsers::parser( int&, const std::string& )
    PARSERS_DECL_FOR_SINGLE( unsigned long long )
    // ========================================================================
    /** parse the <c>double</c> value
     *
     *  @see Gaudi::Parsers::RealGrammar
     *  @param result (output) double result
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    PARSERS_DECL_FOR_SINGLE( double )
    /// @see Gaudi::Parsers::parser( double&, const std::string& )
    PARSERS_DECL_FOR_SINGLE( float )
    /// @see Gaudi::Parsers::parser( double&, const std::string& )
    PARSERS_DECL_FOR_SINGLE( long double )
    // ========================================================================
    /** parse the <c>std::string</c> value
     *
     *  @see Gaudi::Parsers::StringGrammar
     *  @param result (output) string result
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    PARSERS_DECL_FOR_SINGLE( std::string )
    // ========================================================================

    PARSERS_DECL_FOR_LIST( bool )
    PARSERS_DECL_FOR_LIST( char )
    PARSERS_DECL_FOR_LIST( unsigned char )
    PARSERS_DECL_FOR_LIST( signed char )

    PARSERS_DECL_FOR_LIST( int )
    PARSERS_DECL_FOR_LIST( short )
    PARSERS_DECL_FOR_LIST( unsigned short )
    PARSERS_DECL_FOR_LIST( unsigned int )
    PARSERS_DECL_FOR_LIST( long )
    PARSERS_DECL_FOR_LIST( unsigned long )
    PARSERS_DECL_FOR_LIST( long long )
    PARSERS_DECL_FOR_LIST( unsigned long long )

    PARSERS_DECL_FOR_LIST( double )
    PARSERS_DECL_FOR_LIST( float )
    PARSERS_DECL_FOR_LIST( long double )

    PARSERS_DECL_FOR_LIST( std::string )

    // ========================================================================

    PARSERS_DECL_FOR_SET( bool )
    PARSERS_DECL_FOR_SET( char )
    PARSERS_DECL_FOR_SET( unsigned char )
    PARSERS_DECL_FOR_SET( signed char )

    PARSERS_DECL_FOR_SET( int )
    PARSERS_DECL_FOR_SET( short )
    PARSERS_DECL_FOR_SET( unsigned short )
    PARSERS_DECL_FOR_SET( unsigned int )
    PARSERS_DECL_FOR_SET( long )
    PARSERS_DECL_FOR_SET( unsigned long )
    PARSERS_DECL_FOR_SET( long long )
    PARSERS_DECL_FOR_SET( unsigned long long )

    PARSERS_DECL_FOR_SET( double )
    PARSERS_DECL_FOR_SET( float )
    PARSERS_DECL_FOR_SET( long double )

    PARSERS_DECL_FOR_SET( std::string )

    // ========================================================================
    // Advanced parses
    // ========================================================================
    /** parse the <c>std::pair\<double,double\></c> value
     *
     *  @see Gaudi::Parsers::PairGrammar
     *  @see Gaudi::Parsers::RealGrammar
     *  @param result (output) pair of doubles
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    PARSERS_DECL_FOR_PAIR( double, double )
    // ========================================================================
    /** parse the <c>std::pair\<int,int\></c> value
     *
     *  @see Gaudi::Parsers::PairGrammar
     *  @see Gaudi::Parsers::IntGrammar
     *  @param result (output) pair of integers
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    PARSERS_DECL_FOR_PAIR( int, int )
    // ========================================================================
    /** parse the <c>std::vector\<std::pair\<double,double\> \></c> value
     *
     *  @see Gaudi::Parsers::VectorGrammar
     *  @see Gaudi::Parsers::PairGrammar
     *  @see Gaudi::Parsers::RealGrammar
     *  @param result (output) vector with pairs of doubles
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    GAUDI_API StatusCode parse( std::vector<std::pair<double, double>>& result, std::string_view input );
    // ========================================================================
    /** parse the <c>std::vector\<std::pair\<int,int\> \></c> value
     *
     *  @see Gaudi::Parsers::VectorGrammar
     *  @see Gaudi::Parsers::PairGrammar
     *  @see Gaudi::Parsers::IntGrammar
     *  @param result (output) vector with pairs of int
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    GAUDI_API StatusCode parse( std::vector<std::pair<int, int>>& result, std::string_view input );
    // ========================================================================
    // vector< vector< TYPE > >
    // ========================================================================
    /** parse the <c>std::vector\<std::vector\<std::string\> \></c> value
     *
     *  @see Gaudi::Parsers::VectorGrammar
     *  @see Gaudi::Parsers::StringGrammar
     *  @param result (output) vector with vectors of strings
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    GAUDI_API StatusCode parse( std::vector<std::vector<std::string>>& result, std::string_view input );
    // ========================================================================
    /** parse the <c>std::vector\<std::vector\<double\> \></c> value
     *
     *  @see Gaudi::Parsers::VectorGrammar
     *  @see Gaudi::Parsers::RealGrammar
     *  @param result (output) vector with vectors of doubles
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    GAUDI_API StatusCode parse( std::vector<std::vector<double>>& result, std::string_view input );
    // ========================================================================
    /** parse the <c>std::vector\<std::vector\<int\> \></c> value
     *
     *  @see Gaudi::Parsers::VectorGrammar
     *  @see Gaudi::Parsers::IntGrammar
     *  @param result (output) vector with vectors of ints
     *  @param input  (input) the string to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse( std::vector<std::vector<int>>& result, std::string_view input );
    // ========================================================================
    // map< TYPE, TYPE >
    // ========================================================================
    /** parse the <c>std::map\<int , int\></c> value
     *
     *  @see Gaudi::Parsers::MapGrammar
     *  @see Gaudi::Parsers::IntGrammar
     *  @param result (output) map with integer key and double value
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    GAUDI_API StatusCode parse( std::map<int, int>& result, std::string_view input );
    // ========================================================================
    /** parse the <c>std::map\<int , double\></c> value
     *
     *  @see Gaudi::Parsers::MapGrammar
     *  @see Gaudi::Parsers::IntGrammar
     *  @see Gaudi::Parsers::RealGrammar
     *  @param result (output) map with integer key and double value
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    GAUDI_API StatusCode parse( std::map<int, double>& result, std::string_view input );
    // ========================================================================
    /** parse the <c>std::map\<std::string , std::string\></c> value
     *
     *  @see Gaudi::Parsers::MapGrammar
     *  @see Gaudi::Parsers::StringGrammar
     *  @param result (output) map with string key and value
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    GAUDI_API StatusCode parse( std::map<std::string, std::string>& result, std::string_view input );
    GAUDI_API StatusCode parse( std::map<std::string, std::string, std::less<>>& result, std::string_view input );
    // ========================================================================
    /** parse the <c>std::map\<std::string , int\></c> value
     *
     *  @see Gaudi::Parsers::MapGrammar
     *  @see Gaudi::Parsers::StringGrammar
     *  @see Gaudi::Parsers::IntGrammar
     *  @param result (output) map with string key and integer value
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    GAUDI_API StatusCode parse( std::map<std::string, int>& result, std::string_view input );
    // ========================================================================
    /** parse the <c>std::map\<std::string , double\></c> value
     *
     *  @see Gaudi::Parsers::MapGrammar
     *  @see Gaudi::Parsers::StringGrammar
     *  @see Gaudi::Parsers::RealGrammar
     *  @param result (output) map with string key and integer value
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    GAUDI_API StatusCode parse( std::map<std::string, double>& result, std::string_view input );
    // ========================================================================
    /** parse the <c>std::map\<std::string , std::vector\<std::string\> \></c>
     *  value
     *
     *  @see Gaudi::Parsers::MapGrammar
     *  @see Gaudi::Parsers::StringGrammar
     *  @see Gaudi::Parsers::VectorGrammar
     *  @param result (output) map with string value and
     *                 vector of strings as value
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    GAUDI_API StatusCode parse( std::map<std::string, std::vector<std::string>>& result, std::string_view input );
    // ========================================================================
    /** parse the <c>std::map\<std::string , std::vector\<int\> \></c> value
     *
     *  @see Gaudi::Parsers::MapGrammar
     *  @see Gaudi::Parsers::StringGrammar
     *  @see Gaudi::Parsers::VectorGrammar
     *  @see Gaudi::Parsers::IntGrammar
     *  @param result (output) map with string value and
     *                 vector of integers as value
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    GAUDI_API StatusCode parse( std::map<std::string, std::vector<int>>& result, std::string_view input );
    // ========================================================================
    /** parse the <c>std::map\<std::string , std::vector\<double\> \></c> value
     *
     *  @see Gaudi::Parsers::MapGrammar
     *  @see Gaudi::Parsers::StringGrammar
     *  @see Gaudi::Parsers::VectorGrammar
     *  @see Gaudi::Parsers::RealGrammar
     *  @param result (output) map with string value and
     *                 vector of doubles as value
     *  @param input  (input) the string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-14
     */
    GAUDI_API StatusCode parse( std::map<std::string, std::vector<double>>& result, std::string_view input );
    // ========================================================================
    /** parse the <c>std::map\<int,std::string\> \></c> objects
     *
     *  @see Gaudi::Parsers::MapGrammar
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @date 2007-12-06
     */
    GAUDI_API StatusCode parse( std::map<int, std::string>& result, std::string_view input );
    // ========================================================================
    /** parse the <c>std::map\<unsigned int,std::string\> \></c> objects
     *
     *  @see Gaudi::Parsers::MapGrammar
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @date 2007-12-06
     */
    GAUDI_API StatusCode parse( std::map<unsigned int, std::string>& result, std::string_view input );
    // ========================================================================
    /** parse the <c>std::map\<unsigned int,std::string\> \></c> objects
     *
     *  @see Gaudi::Parsers::MapGrammar
     */
    GAUDI_API StatusCode parse( std::map<std::string, unsigned int>& result, std::string_view input );
    // ========================================================================
    /** parse the <c>GaudiUtils::Map\<K, V, M\></c> objects
     *
     *  @see Gaudi::Parsers::MapGrammar
     */
    template <typename K, typename V, typename M>
    GAUDI_API StatusCode parse( GaudiUtils::Map<K, V, M>& result, std::string_view input ) {
      return parse( (M&)result, input );
    }
    // ========================================================================
    /** parse the pair expression (map-component)  " 'name' :value"
     *
     *  @code
     *
     *  const std::string input = "'PackageName':GaudiKernel" ;
     *  std::string name  ;
     *  std::string value ;
     *  StatusCode sc = Gaudi::Parsers::parse ( name , value , input ) ;
     *  if ( sc.isFailure() ) { ... }
     *  std::cout <<  "\tParsed name  is " << name
     *            <<  "\tParsed value is " << value << std::endl
     *  @endcode
     *
     *  @param  name  (output) the parsed name of the component, defined
     *                as 'name' or "name" before the column symbol ":",
     *                the leading and trailing blans are omitted
     *  @param  value (output) the parsed value of the component,
     *                defined as everything after the column symbol ":"
     *                till the end of the string
     *  @param  input (input) string to be parsed
     *  @return status code
     *
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV  ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    GAUDI_API StatusCode parse( std::string& name, std::string& value, std::string_view input );
    // ========================================================================
    /** helper function, needed for implementation of "Histogram Property"
     *  @param histo  the histogram description (output)
     *  @param input the string to be parsed
     *  @return status code
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @date 2007-09-17
     */
    GAUDI_API StatusCode parse( Gaudi::Histo1DDef& histo, std::string_view input );
    // ========================================================================
    /** helper function, needed for implementation of "Histogram Property"
     *  @param histos the map of the histogram descriptions (output)
     *  @param input the string to be parsed
     *  @return status code
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @date 2007-09-17
     */
    GAUDI_API StatusCode parse( std::map<std::string, Gaudi::Histo1DDef>& histos, std::string_view input );
    // ========================================================================
    /** helper function, needed for implementation of map of pairs
     *  @param params the map of pair
     *  @param input the string to be parsed
     *  @return status code
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @date 2009-05-19
     */
    GAUDI_API StatusCode parse( std::map<std::string, std::pair<double, double>>& params, std::string_view input );
    // ========================================================================
    /** helper function, needed for implementation of map of pairs
     *  @param params the map of pair
     *  @param input the string to be parsed
     *  @return status code
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author David BACHER david.bacher@physics.ox.ac.uk
     *  @date 2024-11-06
     */
    GAUDI_API StatusCode parse( std::map<std::string, std::pair<int, int>>& params, std::string_view input );
    // ========================================================================
    /** parser function for C-arrays
     *  @param params C-array
     *  @param input the string to be parsed
     *  @return status code
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-15
     */
    template <class T, unsigned int N>
    StatusCode parse( T ( &result )[N], std::string_view input ) {
      typedef std::vector<T> _Vct;
      // create the temporary vector
      _Vct       tmp;
      StatusCode sc = parse( tmp, input );
      if ( sc.isFailure() ) { return sc; }                   //  RETURN
      if ( N != tmp.size() ) { return StatusCode::FAILURE; } //  RETURN
      //
      std::copy( tmp.begin(), tmp.end(), result );
      //
      return StatusCode::SUCCESS; //  RETURN
    }
    // ========================================================================
    /** parser function for C-strings
     *  @param params C-string
     *  @param input the string to be parsed
     *  @return status code
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-15
     */
    template <unsigned int N>
    StatusCode parse( char ( &result )[N], std::string_view input ) {
      // clear the string
      std::fill_n( result, N, ' ' );
      // create the temporary string
      std::string tmp;
      StatusCode  sc = parse( tmp, input );
      if ( sc.isFailure() ) { return sc; } //  RETURN
      if ( N == tmp.size() ) {
        std::copy( tmp.begin(), tmp.end(), result );
      } else if ( N + 2 == tmp.size() && ( '\'' == tmp[0] || '\"' == tmp[0] ) && ( tmp[0] == tmp[tmp.size() - 1] ) ) {
        std::copy( tmp.begin() + 1, tmp.end() - 1, result );
      } else {
        return StatusCode::FAILURE;
      }
      //
      return StatusCode::SUCCESS; //  RETURN
    }
    // ========================================================================
  } // namespace Parsers
  // ==========================================================================
} //                                                     end of namespace Gaudi
