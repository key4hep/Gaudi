/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/Kernel.h>
#include <GaudiKernel/StatusCode.h>
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace Gaudi {

  /**
   * Transparent hash operator for heterogeneous lookups.
   * @code
   *  std::unordered_set<Gaudi::StringKey, Gaudi::StringKeyHash, std::equal_to<>> set;
   * @endcode
   */
  struct StringKeyHash {
    using is_transparent = void;
    size_t operator()( std::string_view s ) const { return std::hash<std::string_view>{}( s ); }
  };

  /** @class StringKey
   *  Helper class for efficient "key" access for strings.
   *
   *  Multiple lookups of the same key can be speed up by using
   *  a pre-computed key:
   *  @code
   *   std::map<StringKey, double> m = {...};
   *   const StringKey& key("SomeLongKey");
   *
   *   // EFFICIENT:
   *   auto i1 = m.find ( key ) ;
   *
   *   // CAN BE VERY INEFICIENT:
   *   auto i2 = m_find( "SomeLongKey" );
   *  @endcode
   *
   *  The class also supports heterogeneous lookups without the need to construct a
   *  temporary StringKey object:
   *  @code
   *   std::unordered_set<Gaudi::StringKey, Gaudi::StringKeyHash, std::equal_to<>> s = {...};
   *
   *   s.contains("foo");
   *  @endcode
   *
   *  @attention NEVER use the actual hash value for anything stored in
   *  files, as it is not guaranteed that the hashing scheme will remain
   *  the same.
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @author Gerhard Raven (based on stringKey)
   *  @author Frank Winklmeier
   *  @date   2009-04-08
   */
  class GAUDI_API StringKey {
  public:
    /// Default constructor from empty string
    StringKey() : StringKey( std::string_view{} ) {}
    /// constructor from plain C-string, perform hashing
    StringKey( const char* key ) : StringKey( std::string_view{ key } ) {}
    /// constructor from std::string_view, perform hashing
    StringKey( std::string_view key ) : m_hash{ StringKeyHash()( key ) }, m_str( key ) {}
    /// constructor from std::string, perform hashing
    StringKey( std::string key ) : m_hash{ StringKeyHash()( key ) }, m_str( std::move( key ) ) {}

    /// the actual string
    const std::string& str() const { return m_str; }
    /// implicit cast to std::string
    operator const std::string&() const { return m_str; }
    /// implicit cast to std::string_view
    operator std::string_view() const { return m_str; }
    /// empty key?
    bool empty() const { return m_str.empty(); }
    /// empty key?
    bool operator!() const { return empty(); }

    /// Compiler generated equality operator first comparing hash, then string (see below)
    bool operator==( const StringKey& ) const = default;
    /// Compiler generated spaceship operator first comparing hash, then string (see below)
    auto operator<=>( const StringKey& ) const = default;

    /// Comparison to other string-like types
    bool operator==( const char* rhs ) const { return m_str == rhs; }
    bool operator==( std::string_view rhs ) const { return m_str == rhs; }
    bool operator==( const std::string& rhs ) const { return m_str == rhs; }

    /** the actual access to the hash
     *  @attention NEVER use the actual hash value for anything stored in
     *             files, as it is not guaranteed that the hashing scheme
     *             will remain the same.
     *             The two reason for this function are:
     *    - transparent usage of this object for hashmap-like containers
     *    - Python
     *  @reutrn the actual hash value
     */
    std::size_t __hash__() const { return m_hash; }
    /// the representation of the object
    const std::string& __str__() const; // the representation of the object
    /// the representation of the object
    std::string __repr__() const; // the representation of the object
    /// equality operator for python
    bool __eq__( const StringKey& right ) const;
    /// equality operators for python
    bool __eq__( std::string_view right ) const;
    /// non-equality operator for python
    bool __neq__( const StringKey& right ) const;
    /// non-equality operator for python
    bool __neq__( const std::string_view right ) const;

    /// string representation (for properties)
    std::string toString() const; // string representation (for properties)

    // interoperability with std::string and const char*
    friend std::string operator+( const std::string& lhs, const Gaudi::StringKey& rhs ) { return lhs + rhs.str(); }
    friend std::string operator+( const char* lhs, const Gaudi::StringKey& rhs ) { return lhs + rhs.str(); }
    friend std::string operator+( const Gaudi::StringKey& lhs, const std::string& rhs ) { return lhs.str() + rhs; }
    friend std::string operator+( const Gaudi::StringKey& lhs, const char* rhs ) { return lhs.str() + rhs; }

  private:
    // !!!
    // Do not change the order of these two members. The compiler-generated
    // comparison operators will first compare the hash (fast) and then the full string.
    // !!!

    /// the hash
    std::size_t m_hash;

    /// the actual string
    std::string m_str;
  };

  /** hash-function: heeded for boost::hash
   *  @attention NEVER use the actual hash value for anything stored in
   *  files, as it is not guaranteed that the hashing scheme will remain
   *  the same. The only reason for this function is Python and
   *  transparent usage of this object for hashmap-like containers
   *  @see Gaudi::Hash
   *  @return the actual hash value
   *  @author Vanya BELYAEV Iavn.Belyaev@nikhef.nl
   *  @date 2009-10-07
   */
  inline std::size_t hash_value( const Gaudi::StringKey& key ) { return key.__hash__(); }
} // namespace Gaudi

// Streaming  value -> string
namespace Gaudi {
  namespace Utils {
    /** send the object to stream (needed to use it as property)
     *  @see Gaudi::StringKey
     *  @see Gaudi::Utils::toString
     *  @see Gaudi::Utils::toStream
     *  @param key (INPUT) the object to be printed
     *  @param s   (OUTPUT) the stream
     *  @return the stream
     *  @author Vanya BELYAEV Iavn.Belyaev@nikhef.nl
     *  @date 2009-10-07
     */
    GAUDI_API std::ostream& toStream( const Gaudi::StringKey& key, std::ostream& s );
  } // namespace Utils
  /** printout of the object
   *  reply on the native printout for the string
   *  @author Vanya BELYAEV Iavn.Belyaev@nikhef.nl
   *  @date 2009-10-07
   */
  inline std::ostream& operator<<( std::ostream& o, const Gaudi::StringKey& key ) { return o << key.str(); }
} // namespace Gaudi

// Parsing : string -> value
namespace Gaudi {
  namespace Parsers {
    /** parse the key from the string
     *  @see Gaudi::Parsers
     *  @see Gaudi::Parsers::parse
     *  @see Gaudi::StringKey
     *  @attention: this function is needed to use it as property
     *  @param result (OUTPUT) the parsing result
     *  @param input the input string
     *  @return status code
     */
    GAUDI_API StatusCode parse( Gaudi::StringKey& result, std::string_view input );
    /** parse the vector of keys from the string
     *  @see Gaudi::Parsers
     *  @see Gaudi::Parsers::parse
     *  @see Gaudi::StringKey
     *  @attention: this function is needed to use it as property
     *  @param result (OUTPUT) the parsing result
     *  @param input the input string
     *  @return status code
     */
    GAUDI_API StatusCode parse( std::vector<Gaudi::StringKey>& result, std::string_view input );
  } // namespace Parsers
} // namespace Gaudi

namespace std {
  /// specialization of hash function used in C++11 collections like
  /// std::unordered_map
  /// \see https://its.cern.ch/jira/browse/GAUDI-973
  template <>
  struct hash<Gaudi::StringKey> {
    std::size_t operator()( Gaudi::StringKey const& s ) const { return hash_value( s ); }
  };
} // namespace std
