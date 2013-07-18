#ifndef GAUDIKERNEL_STRINGKEY_H 
#define GAUDIKERNEL_STRINGKEY_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL 
// ============================================================================
#include <iosfwd>
#include <string>
#include <functional>
#include <algorithm>
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Kernel.h"
// ============================================================================
namespace Gaudi 
{
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
  class GAUDI_API StringKey 
  {
  public:
    // ========================================================================
    /// constructor from the string, perform hashing
    StringKey ( const std::string& key = "" ) ; // constructor, perform hashing
    // ========================================================================
  public:
    // ========================================================================
    /// the actual string
    const std::string&      str() const { return m_str ;  }
    /// implicit cast to std::string
    operator const std::string&() const { return str() ;  }
    /// empty key?
    bool empty    () const { return m_str.empty() ; }
    /// empty key?
    bool operator!() const { return       empty() ; }      
    // ========================================================================
  public: // equality 
    // ========================================================================
    /** equality                                                            Key
     *  for efficiency reason compare the hash-values first 
     */
    bool operator == ( const StringKey&   o ) const 
    { return m_hash == o.m_hash && m_str == o.m_str ; }
    /** equality, without hashing                                        string
     *  rely on the native string equality
     */ 
    bool operator == ( const std::string& o ) const { return m_str == o ; }
    // ========================================================================
  public: // non-equality
    // ========================================================================
    /// non equality                                                      Key
    bool operator!=( const StringKey&   o ) const { return !(*this==o) ; }
    /// non-equality                                                   string
    bool operator!=( const std::string& o ) const { return !(*this==o) ; }
    // ========================================================================
  public: // ordering
    // ========================================================================
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
    bool operator < ( const StringKey& o ) const 
    { return m_hash == o.m_hash ? m_str < o.m_str : m_hash < o.m_hash   ; }
    /// greater                                                             key
    bool operator > ( const StringKey& o ) const { return    o < *this  ; }
    /// less or equal                                                       key
    bool operator <=( const StringKey& o ) const { return !(*this > o ) ; }
    /// greater or equal                                                    key
    bool operator >=( const StringKey& o ) const { return !(*this < o ) ; }
    // ========================================================================
  public:  // few helper methods for indirect usage, mainly for Python
    // ========================================================================
    /** the actual access to the hash
     *  @attention NEVER use the actual hash value for anything stored in 
     *             files, as it is not guaranteed that the hashing scheme 
     *             will remain the same. 
     *             The two reason for this function are:
     *    - transparent usage of this object for hashmap-like containers
     *    - Python  
     *  @reutrn the actual hash value 
     */
    std::size_t __hash__  () const { return m_hash ; }
    // ========================================================================
    /// the representation of the object
    std::string __str__   () const ; // the representation of the object
    /// the representation of the object
    std::string __repr__  () const ; // the representation of the object
    /// equality operator for python 
    bool        __eq__    ( const StringKey&   right ) const ;
    /// equality operators for python 
    bool        __eq__    ( const std::string& right ) const ;    
    /// non-equality operator for python 
    bool        __neq__   ( const StringKey&   right ) const ;
    /// non-equality operator for python 
    bool        __neq__   ( const std::string& right ) const ;    
    // ========================================================================
  public:
    // ========================================================================
    /// string representation (for properties)
    std::string toString  () const ;  // string representation (for properties)
    // ========================================================================
  private:
    // ========================================================================
    /// the actual string:
    std::string m_str  ;                                   // the actual string
    /// the hash:
    std::size_t m_hash ;                                   //          the hash
    // ========================================================================
  };
  // ==========================================================================
  /** equality operator with C-arrays 
   *  @author Vanya BELYAEV Iavn.Belyaev@nikhef.nl
   *  @date 2009-10-07
   */      
  template <unsigned int N>  
  inline bool operator== 
    ( const Gaudi::StringKey&   key1     , 
      const char               (&key2)[N] ) 
  { 
    return 
      key1.str().size() == N && 
      std::equal ( key2 , key2 + N , key1.str().begin() ) ; 
  }
  // ==========================================================================
  /** non-equality operator with C-arrays 
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-10-07
   */
  template <unsigned int N>
  inline bool operator!= 
  ( const Gaudi::StringKey&   key1     , 
    const char              (&key2)[N] ) 
  { return ! ( key1 == key2 ) ; }
  // ==========================================================================
  /** "right" form of equality operator
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-10-07
   */      
  inline bool operator== 
    ( const std::string&        key1 , 
      const Gaudi::StringKey&   key2 ) 
  { return key2 == key1 ; }
  // ==========================================================================
  /** "right" form of equality operator
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-10-07
   */      
  template <unsigned int N>
  inline bool operator== 
    ( const char              (&key1 )[N] , 
      const Gaudi::StringKey&   key2      )
  { return key2 == key1 ; }
  // ==========================================================================
  /** "right" form of non-equality operator
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-10-07
   */      
  inline bool operator!= 
  ( const std::string&          key1 , 
    const Gaudi::StringKey&     key2 ) 
  { return key2 != key1 ; }
  // ==========================================================================
  /** "right" form of non-equality operator
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-10-07
   */      
  template <unsigned int N>
  inline bool operator!= 
  ( const char              (&key1)[N] ,
    const Gaudi::StringKey&   key2     ) { return key2 != key1 ; }
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
  inline std::size_t hash_value ( const Gaudi::StringKey& key )
  { return key.__hash__ () ; }
  // ==========================================================================
} //                                                     end of namespace Gaudi 
// ============================================================================
// Streaming  value -> string 
// ============================================================================
namespace Gaudi 
{
  // ==========================================================================
  namespace Utils 
  {
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
    GAUDI_API std::ostream& toStream 
    ( const Gaudi::StringKey& key , 
      std::ostream&             s   ) ;
    // ========================================================================
  } //                                            end of namespace Gaudi::Utils 
  // ==========================================================================
  /** printout of the object 
   *  reply on the native printout for the string
   *  @author Vanya BELYAEV Iavn.Belyaev@nikhef.nl
   *  @date 2009-10-07
   */
  inline std::ostream& operator<< 
    ( std::ostream&           o   , 
      const Gaudi::StringKey& key ) { return  o << key.str() ; }  
  // ==========================================================================
} //                                                     end of namespace Gaudi 
// ============================================================================
// Parsing : string -> value 
// ============================================================================
namespace Gaudi 
{
  // ==========================================================================
  namespace Parsers 
  {
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
    GAUDI_API StatusCode parse
    ( Gaudi::StringKey&  result , 
      const std::string& input  ) ;
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
    GAUDI_API StatusCode parse
    ( std::vector<Gaudi::StringKey>&  result , 
      const std::string&              input  ) ;
    // ========================================================================
  } //                                          end of namespace Gaudi::Parsers 
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// The END 
// ============================================================================
#endif // GAUDIKERNEL_STRINGKEY_H
// ============================================================================
