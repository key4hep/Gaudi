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
#ifndef GAUDIKERNEL_STRINGKEY_H
#define GAUDIKERNEL_STRINGKEY_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <algorithm>
#include <functional>
#include <iosfwd>
#include <string>
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StatusCode.h"
// ============================================================================
namespace Gaudi {
  // ==========================================================================
  /** @class StringKey GaudiKernel/StringKey.h
   *  The helper class to represent the efficient "key" for access.
   *  Essentially it is a bit modified version ("boost-free") of the
   *  original class
   *  stringKey by Gerhard Raven, which is heavily used now in HLT
   *
   *  @attention NEVER use the actual hash value for anything stored in
   *  files, as it is not guaranteed that the hashing scheme will remain
   *  the same.
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date   2009-04-08
   */
  class GAUDI_API StringKey {
  public:
    // ========================================================================
    /// constructor from plain C-string, perform hashing
    StringKey( const char* key = "" ) : StringKey{std::string{key}} {}
    //
    /// constructor from std::string, perform hashing
    StringKey( std::string key ); // constructor, perform hashing
    // ========================================================================
  public:
    // ========================================================================
    /// the actual string
    const std::string& str() const { return m_str; }
    /// implicit cast to std::string
    operator const std::string&() const { return str(); }
    /// empty key?
    bool empty() const { return m_str.empty(); }
    /// empty key?
    bool operator!() const { return empty(); }
    // ========================================================================
  public:
    // ========================================================================
    /** equality                                                            Key
     *  for efficiency reason compare the hash-values first
     */
    friend bool operator==( const StringKey& lhs, const StringKey& rhs ) {
      return lhs.m_hash == rhs.m_hash && lhs.m_str == rhs.m_str;
    }
    /** equality, without hashing                                        string
     *  rely on the native string equality
     */
    friend bool operator==( const StringKey& lhs, const std::string& rhs ) { return lhs.m_str == rhs; }
    friend bool operator==( const std::string& lhs, const StringKey& rhs ) { return rhs == lhs; }
    friend bool operator==( const StringKey& lhs, std::string_view rhs ) { return lhs.m_str == rhs; }
    friend bool operator==( std::string_view lhs, const StringKey& rhs ) { return rhs == lhs; }
    // ========================================================================
  public: // non-equality
    // ========================================================================
    /// non equality                                                      Key
    friend bool operator!=( const StringKey& lhs, const StringKey& rhs ) { return !( lhs == rhs ); }
    /// non-equality                                                   string
    friend bool operator!=( const StringKey& lhs, std::string_view rhs ) { return !( lhs == rhs ); }
    friend bool operator!=( const StringKey& lhs, const std::string& rhs ) { return !( lhs == rhs ); }
    friend bool operator!=( std::string_view lhs, const StringKey& rhs ) { return !( lhs == rhs ); }
    friend bool operator!=( const std::string& lhs, const StringKey& rhs ) { return !( lhs == rhs ); }
    // ========================================================================
    // ordering
    // ========================================================================
  public:
    /** less                                                                key
     *  It can be used as a key for std::map, e.g.
     *  <code>std::map<StringKey,double></code>
     *  Note that with such maps one can gain if using prehashed key:
     *  @code
     *
     *   typedef std::map<StringKey,double> MAP ;
     *
     *   const StringKey& key = ...  ;
     *
     *   const MAP& m = ... ;
     *
     *   // EFFICIENT:
     *  MAP::const_iterator i1 = m.find ( key ) ;
     *
     *  // CAN BE VERY INEFICIENT:
     *  MAP::const_iterator i2 = m_find( "SomeLongKey,_e.g._TES_Locaiton" );
     *
     *  @endcode
     */
    friend bool operator<( const StringKey& lhs, const StringKey& rhs ) {
      return lhs.m_hash == rhs.m_hash ? lhs.m_str < rhs.m_str : lhs.m_hash < rhs.m_hash;
    }
    /// greater                                                             key
    friend bool operator>( const StringKey& lhs, const StringKey& rhs ) { return rhs < lhs; }
    /// less or equal                                                       key
    friend bool operator<=( const StringKey& lhs, const StringKey& rhs ) { return !( lhs > rhs ); }
    /// greater or equal                                                    key
    friend bool operator>=( const StringKey& lhs, const StringKey& rhs ) { return !( lhs < rhs ); }
    // ========================================================================
    // few helper methods for indirect usage, mainly for Python
    // ========================================================================
  public:
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
    // ========================================================================
    /// the representation of the object
    std::string __str__() const; // the representation of the object
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
    // ========================================================================
  public:
    // ========================================================================
    /// string representation (for properties)
    std::string toString() const; // string representation (for properties)
    // ========================================================================
  private:
    // ========================================================================
    /// the actual string:
    std::string m_str; // the actual string
    /// the hash:
    std::size_t m_hash; //          the hash
    // ========================================================================
  };
  // ==========================================================================
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
  // ==========================================================================
} //                                                     end of namespace Gaudi
// interoperability with std::string and const char*
inline std::string operator+( const std::string& lhs, const Gaudi::StringKey& rhs ) { return lhs + rhs.str(); }
inline std::string operator+( const char* lhs, const Gaudi::StringKey& rhs ) { return lhs + rhs.str(); }
inline std::string operator+( const Gaudi::StringKey& lhs, const std::string& rhs ) { return lhs.str() + rhs; }
inline std::string operator+( const Gaudi::StringKey& lhs, const char* rhs ) { return lhs.str() + rhs; }
// ============================================================================
// Streaming  value -> string
// ============================================================================
namespace Gaudi {
  // ==========================================================================
  namespace Utils {
    // ========================================================================
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
    // ========================================================================
  } // namespace Utils
  // ==========================================================================
  /** printout of the object
   *  reply on the native printout for the string
   *  @author Vanya BELYAEV Iavn.Belyaev@nikhef.nl
   *  @date 2009-10-07
   */
  inline std::ostream& operator<<( std::ostream& o, const Gaudi::StringKey& key ) { return o << key.str(); }
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// Parsing : string -> value
// ============================================================================
namespace Gaudi {
  // ==========================================================================
  namespace Parsers {
    // ========================================================================
    /** parse the key from the string
     *  @see Gaudi::Parsers
     *  @see Gaudi::Parsers::parse
     *  @see Gaudi::StringKey
     *  @attention: this function is needed to use it as property
     *  @param result (OUTPUT) the parsing result
     *  @param input the input string
     *  @return status code
     */
    GAUDI_API StatusCode parse( Gaudi::StringKey& result, const std::string& input );
    // ========================================================================
    /** parse the vector of keys from the string
     *  @see Gaudi::Parsers
     *  @see Gaudi::Parsers::parse
     *  @see Gaudi::StringKey
     *  @attention: this function is needed to use it as property
     *  @param result (OUTPUT) the parsing result
     *  @param input the input string
     *  @return status code
     */
    GAUDI_API StatusCode parse( std::vector<Gaudi::StringKey>& result, const std::string& input );
    // ========================================================================
  } // namespace Parsers
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
namespace std {
  /// specialization of hash function used in C++11 collections like
  /// std::unordered_map
  /// \see https://its.cern.ch/jira/browse/GAUDI-973
  template <>
  struct hash<Gaudi::StringKey> {
    inline std::size_t operator()( Gaudi::StringKey const& s ) const { return hash_value( s ); }
  };
} // namespace std
// ============================================================================
#endif // GAUDIKERNEL_STRINGKEY_H
