// $Id: VectorMap.h,v 1.11 2007/05/24 14:39:11 hmd Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.11 $
// ============================================================================
#ifndef GAUDIKERNEL_VECTORMAP_H
#define GAUDIKERNEL_VECTORMAP_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <utility>
#include <functional>
#include <vector>
#include <algorithm>
#include <ostream>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/MapBase.h"

// For parsers
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/StringKey.h"
// ============================================================================
namespace GaudiUtils
{
  // ==========================================================================
  /** @class VectorMap GaudiKernel/VectorMap.h
   *
   *  A bit modified version of 'Loki::AssocVector' associative
   *  vector from Loki library by Andrei Alexandrescu
   *
   *  The number of "non-const" operations is reduced,
   *  e.g. all non-const iterators are not exported,
   *  therefore it is almost impossible e.g. externally
   *  re-sort the underlying sorted container.
   *
   *  ---------------------------
   *  The nominal CPU performance:
   *  ---------------------------
   *    Container insertion: O(N)
   *    Container look-up:   O(log(N)) (a'la std::map, but a bit faster)
   *
   *  It could be used as a "light" and good alternative
   *  for std::map associative container, in the case of
   *  relatively rare insertion and frequent look-up.
   *
   *  Due to helper base class Gaudi::Utils::MapBase, this class
   *  is "python-friendly", and one can perform all python
   *  manipulaitons
   *  in intuitive way:
   *  @code
   *
   *    >>> m = ...        ## get the map
   *    >>> print m        ## print the map a'la python class dict
   *    ...
   *    >>> for key in m :  print key, m[key]   ## iteration over the map
   *    ...
   *    >>> for key,value in m.iteritems() : print key, value
   *    ...
   *    >>> keys   = m.keys()                     ## get the list of keys
   *    >>> values = m.values ()                  ## get the list of values
   *    >>  items  = m.items  ()                  ## get the list of items
   *
   *    >>> if 'one' in m           ## check the presence of the key in map
   *
   *    >>>  v = m.get(key', None) ## return m[key] for existing key, else None
   *
   *    >>>  del m[key]      ## erase the key form the map
   *
   *    >>> value m[key]     ## unchecked access through the key
   *    ...
   *    >>> m.update( key, value ) ## update/insert key/value pair
   *
   *   @endcode
   *
   *   @attention The syntax can be drastically simplified, if one
   *              redefines the __setitem__ attribute:
   *
   *   @code
   *
   *    >>> type(m).__setitem__ = Gaudi.Utils.MapBase.__setitem__
   *
   *    >>> m[key] = value  ## much more intuitive semantics for key insertion
   *
   *   @endcode
   *   In a similar way <c>__getitem__</c> and <c>__delitem__</c> methods
   *   can be redefind
   *
   *   To avoid the unnesessary expansion of dictionaries
   *   it is highly recommended to exclude from dictionary the following methods:
   *     - lower_bound
   *     - upper_bound
   *     - equal_range
   *     - insert
   *
   *   @see Gaudi::Utils::MapBase
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date   2005-07-23
   */
  template
  <
    class KEY                                                 ,
    class VALUE                                               ,
    class KEYCOMPARE=std::less<const KEY>                     ,
    class ALLOCATOR=std::allocator<std::pair<KEY,VALUE> >
    >
  class VectorMap : public Gaudi::Utils::MapBase
  {
  public:
    // ========================================================================
    /// the actual type of key
    typedef KEY                                    key_type         ;
    /// the actual type of value
    typedef VALUE                                  mapped_type      ;
    /// comparison of keys
    typedef KEYCOMPARE                             key_compare      ;
    /// the actual storage item
    typedef std::pair<key_type,mapped_type>        value_type       ;
    // ========================================================================
  public:
    // ========================================================================
    /// allocator (could be useful for optimizations)
    typedef ALLOCATOR                              allocator_type   ;
    /// the types to conform STL
    typedef typename ALLOCATOR::const_reference    reference        ;
    /// the types to conform STL
    typedef typename ALLOCATOR::const_reference    const_reference  ;
    /// the types to conform STL
    typedef typename ALLOCATOR::size_type          size_type        ;
    /// the types to conform STL
    typedef typename ALLOCATOR::difference_type    difference_type  ;
    // ========================================================================
  public:
    // ========================================================================
    /// the actual storage container (no export)
    typedef std::vector<value_type,allocator_type> _vector          ;
    // ========================================================================
  protected:
    // ========================================================================
    /// the regular iterator  (no export)
    typedef typename _vector::iterator             _iterator        ;
    // ========================================================================
  public:
    // ========================================================================
    /// visible const_iterator (exported)
    typedef typename _vector::const_iterator       iterator         ;
    /// visible const_iterator (exported)
    typedef typename _vector::const_iterator       const_iterator   ;
    /// visible reverse const_iterator (exported)
    typedef std::reverse_iterator<iterator>        reverse_iterator ;
    /// visible reverse const_iterator (exported)
    typedef std::reverse_iterator<const_iterator>  const_reverse_iterator ;
    /// visible iterator pait
    typedef std::pair<iterator,iterator>           iterators        ;
    /// visible iterator pait
    typedef std::pair<iterator,bool>               result_type      ;
    // ========================================================================
  public:
    // ========================================================================
    /** @struct _compare_type
     *  The actual structure used to compare the elements
     *  Only "key" is important for comparison
     */
    struct _compare_type : public key_compare
    {
    public:
      // ======================================================================
      /// constructor from the key-comparison criteria
      _compare_type ( const key_compare& cmp ) : key_compare ( cmp ) {}
      /// default constructor
      _compare_type ()                         : key_compare (     ) {}
      /// compare keys: use key_compare
      bool operator () ( const key_type&  k1 , const key_type&   k2 ) const
      { return this->key_compare::operator() ( k1 , k2 ) ; }
      /// compare pairs (key,mapped): use compare by keys
      bool operator() ( const value_type& v1 , const value_type& v2 ) const
      { return operator() ( v1.first, v2.first ); }
      /// compare key and pair (key,mapped): use compare by keys
      bool operator() ( const key_type&   k  , const value_type& v  ) const
      { return operator() ( k , v.first ) ; }
      /// compare pair (key,mapped) and the key: use compare by keys
      bool operator() ( const value_type& v  , const key_type  & k  ) const
      { return operator() ( v.first , k ) ; }
      // ======================================================================
    };
    // ========================================================================
    /// the actual comparison criteria for valye_type objects
    typedef _compare_type                            compare_type   ;
    // ========================================================================
  public:
    // ========================================================================
    // sequential access  (only const-versions!)
    // ========================================================================
    /// "begin"  iterator for sequential access (const-only version!)
    iterator         begin  () const { return m_vct . begin  () ; }
    /// "end"    iterator for sequential access (const-only version!)
    iterator         end    () const { return m_vct . end    () ; }
    /// "rbegin" iterator for sequential access (const-only version!)
    reverse_iterator rbegin () const { return m_vct . rbegin () ; }
    /// "rend"   iterator for sequential access (const-only version!)
    reverse_iterator rend   () const { return m_vct . rend   () ; }
    // ========================================================================
    // list operations : erase & insert
    // ========================================================================
    /** erase the element using the iterator
     *  @param pos position of the element to be erased
     */
    void erase  ( iterator pos ) { m_vct.erase ( iter ( pos ) ) ; }
    // ========================================================================
    /** erase the element using the key
     *
     *  @code
     *
     *  GaudiUtils::VectorMap<K,V> m = ... ;
     *
     *  ...
     *  K key = ... ;
     *
     *  std::cout << " # of erased elements "
     *            << m.erase ( key ) << std::endl ;
     *  @endcode
     *
     *  @param key key for the element to be erased
     *  @return number of erased elements (0 or 1)
     */
    size_type erase  ( const key_type&    key    )
    {
      iterator pos = find ( key ) ;
      if ( end() == pos ) { return 0 ; }
      erase ( pos ) ;
      return 1 ;
    }
    // ========================================================================
    /** erase the sequence of elements using the iterators
     *  @param first begin iterator of sub-sequence to be erased
     *  @param end   end iterator of the sub_sequence to be erased
     *  @return number of erased elements
     */
    size_type erase  ( iterator           first   ,
                       iterator           last    )
    {
      m_vct.erase ( iter ( first ) , iter ( last )  ) ;
      return last - first ;
    }
    // ========================================================================
    /** erase the sequence of elements using the sequence of keys
     *
     *  @code
     *
     *  GaudiUtils::VectorMap<K,V> m = ... ;
     *
     *  // some sequence of keys, to be removed
     *  KEYS keys = ... ;
     *
     *  std::cout
     *   << " # keys to be removed: " << keys.size()
     *   << " # keys removed: " << m.erase( keys.begin() , keys.end() )
     *   << std::endl ;
     *
     *  @endcode
     *
     *  @param first begin-iterator for the sequence of keys
     *  @param last    end-iterator for the sequence of keys
     *  @return number of erased elements
     */
    template <class TYPE>
    size_type erase  ( TYPE first , TYPE last )
    {
      size_type res = 0 ;
      for ( ; first != last ; ++first ) { res += erase ( *first ) ; }
      return res ;
    }
    // ========================================================================
    /** insert the (key,value) pair into the container
     *
     *  @attention there is no replacement for the existing key!
     *
     *  It is STL-compliant behavior for associative containers.
     *
     *  @code
     *
     *  GaudiUtils::VectorMap<K,V> m ;
     *
     *  K key  = ... ;
     *  V val1 = ... ;
     *  V val2 = ... ;
     *
     *  std::cout
     *     << " Before insert: " << m[key]                  // should be: V()
     *     << std::end ;
     *
     *  // insert the value into the map:
     *  const bool inserted1 = m.insert( key , val1 ).second ;
     *  std::cout
     *     << " 1st insert: "
     *     << Gaudi::Utils::toString( inserted1 )           // should be: "True"
     *     << " value: " << m[key]                          // should be: val1
     *     << std::endl ;
     *
     *  // try to re-insert another value with the same key:
     *  const bool inserted2 = m.insert( key , val2 ).second ;
     *  std::cout
     *     << " 2nd insert: "
     *     << Gaudi::Utils::toString( inserted2 )           // should be: "False"
     *     << " value: " << m[key]                          // should be: val1
     *     << std::endl ;
     *
     *  @endcode
     *
     *  @param  key    key value to be inserted
     *  @param  mapped value to be associated with the key
     *  @return position of the inserted elements with the flag
     *          which allows to distinguish the actual insertion
     */
    result_type insert
    ( const key_type&    key    ,
      const mapped_type& mapped  )
    { return insert ( value_type ( key , mapped ) ) ; }
    // ========================================================================
    /** insert the (key,value) pair into the container
     *
     *  @attention there is no replacement for the existing element!
     *
     *  It is STL-compliant behavior for associative containers.
     *
     *  @code
     *
     *  GaudiUtils::VectorMap<K,V> m ;
     *
     *  K key  = ... ;
     *  V val1 = ... ;
     *  V val2 = ... ;
     *
     *  std::cout
     *     << " Before insert: " << m[key]                  // should be: V()
     *     << std::end ;
     *
     *  // insert the value into the map:
     *  const bool inserted1 = m.insert ( std::make_pair( key , val1 ) ).second ;
     *  std::cout
     *     << " 1st insert: "
     *     << Gaudi::Utils::toString( inserted1 )           // should be: "True"
     *     << " value: " << m[key]                          // should be: val1
     *     << std::endl ;
     *
     *  // try to re-insert another value with the same key:
     *  const bool inserted2 = m.insert ( std::make_pair( key , val2 ) ).second ;
     *  std::cout
     *     << " 2nd insert: "
     *     << Gaudi::Utils::toString( inserted2 )           // should be: "False"
     *     << " value: " << m[key]                          // should be: val1
     *     << std::endl ;
     *
     *  @endcode
     *
     *  @param  value value to be inserted
     *  @return position of the inserted elements with the flag
     *          which allows to distinguish the actual insertion
     */
    result_type insert
    ( const value_type&  value  )
    {
      bool found = true ;
      _iterator result = lower_bound ( value.first ) ;
      if ( end() == result || compare( value.first , result -> first ) )
      { result = m_vct.insert ( result , value ) ; found = false ; }
      return result_type ( iter ( result ) , !found ) ;
    }
    // ========================================================================
    /** insert the element with some guess about its new position
     *  With the right guess the method could be  more efficient
     *  @attention there is no replacement for the existing element!
     *  @param  pos the guess about position where to insert the element
     *  @param  value value to be inserted
     *  @return position of the inserted elements with the flag
     *          which indicated the actual insertion
     */
    result_type insert
    ( iterator           pos    ,
      const value_type&  value  )
    {
      if ( pos != end() && compare ( *pos , value ) &&
           ( pos == end() - 1 ||
              ( !compare ( value , *( pos + 1 ) )
                && compare ( *( pos + 1 ) , value ) ) ) )
      { return result_type( m_vct.insert ( iter ( pos ) , value ) , true ) ; }
      return insert ( value ) ;
    }
    // ========================================================================
    /** insert the (key,value) pair into the container
     *  With the right guess the method could be more efficient
     *  @attention there is no replacement for the existing element!
     *  @param  pos    the guess about position where to insert the element
     *  @param  key    key value to be inserted
     *  @param  mapped value to be associated with the key
     *  @return position of the inserted elements with the flag
     *          which indicated the actual insertion
     */
    result_type insert
    ( iterator           pos    ,
      const key_type&    key    ,
      const mapped_type& mapped )
    { return insert ( pos , value_type ( key , mapped ) ) ; }
    // ========================================================================
    /** insert the sequence of elements into the container
     *  @attention there is no replacement for the existing element!
     *  @param first the begin iterator of the sequence
     *  @param last the end iterator of the sequence
     */
    template <class PAIRS>
    void insert
    ( PAIRS first ,
      PAIRS last  )
    { for ( ; first != last ; ++first ) { insert ( *first ) ; } }
    // ========================================================================
    /** insert into the container the elements from
     *  2 "parallel" sequences
     *  @attention there is no replacement for the existing element!
     *  @param kf the begin iterator of the sequence of keys
     *  @param kl the end iterator of the sequence of keys
     *  @param vf the begin iterator of the sequence of values
     */
    template <class KEYS, class VALUES> void insert
    ( KEYS   kf ,
      KEYS   kl ,
      VALUES vf )
    { for ( ; kf != kl ; ++kf, ++vf ) { insert ( *kf , *vf ) ; } }
    // ========================================================================
    // map operations: lookup, count, ...
    // ========================================================================
    /** find the element by key
     *
     *  @code
     *
     *  typedef GaudiUtils::VectorMap<K,V> Map ;
     *
     *  Map m  = ... ;
     *
     *  K key = ...;
     *
     *  // Is key in the container?
     *  Map::iterator ifound = m.find( key ) ;
     *
     *  if ( m.end() != ifound )
     *   {
     *     std::cout << "The value is : " << ifound->second << std::endl ;
     *   }
     *
     *  @endcode
     *
     *  @param key key to be searched
     *  @return iterator to the element position in the container
     */
    iterator find ( const key_type& key ) const
    {
      iterator res = lower_bound ( key ) ;
      if ( end() != res && compare ( key , res->first ) )
      { res = end(); }
      return res ;
    }
    // ========================================================================
    /** count number of elements with the certain key
     *
     *  @code
     *
     *  GaudiUtils::VectorMap<K,V> m = ... ;
     *
     *  K key = ... ;
     *
     *  std::cout << " # of elements for the key: " << m.count(key) << std::end ;
     *
     *  @endcode
     *
     *  @param key key to be searched
     *  @return number of elements with the given key (0 or 1)
     */
    size_type count ( const key_type& key ) const
    { return end() == find ( key ) ? 0 : 1 ; }
    // ========================================================================
    iterator  lower_bound ( const key_type& key ) const
    { return std::lower_bound ( begin () , end () , key , compare () ) ; }
    iterator  upper_bound ( const key_type& key ) const
    { return std::upper_bound ( begin () , end () , key , compare () ) ; }
    iterators equal_range ( const key_type& key ) const
    { return std::equal_range ( begin () , end () , key , compare () ) ; }
    // ========================================================================
    // general container operations :
    // ========================================================================
    /// empty container ?
    bool      empty    () const { return m_vct . empty    () ; }
    /// number of elements
    size_type size     () const { return m_vct . size     () ; }
    /// maximal allowed size
    size_type max_size () const { return m_vct . max_size () ; }
    /// clear the container
    void clear    ()                { m_vct.clear   ()       ; }
    /// reserve the space in the container for at least 'num' elements
    void reserve  ( size_type num ) { m_vct.reserve ( num )  ; }
    // ========================================================================
    /// swap function, which 'swaps' the content of two containers
    void swap     ( VectorMap& other )
    {
      std::swap ( m_vct , other.m_vct ) ;
    }
    // ========================================================================
    // The basic comparison operators for container
    // ========================================================================
    /// comparison criteria for containers
    bool operator== ( const VectorMap& other ) const
    { return m_vct == other.m_vct ; }
    /// comparison criteria for containers
    bool operator<  ( const VectorMap& other ) const
    { return m_vct <  other.m_vct ; }
    // ========================================================================
    // The derived comparison operators for container
    // ========================================================================
    friend bool operator>  ( const VectorMap& left  ,
                             const VectorMap& right )
    { return    right < left     ; }
    friend bool operator!= ( const VectorMap& left  ,
                             const VectorMap& right )
    { return !( left == right  ) ; }
    friend bool operator>= ( const VectorMap& left  ,
                             const VectorMap& right )
    { return !( left  <  right ) ; }
    friend bool operator<= ( const VectorMap& left  ,
                             const VectorMap& right )
    { return !( right <  left  ) ; }
    // ========================================================================
    /** forced insertion of the key/mapped pair
     *  The method acts like "insert" but it *DOES*
     *  overwrite the existing mapped value.
     *
     *  @attention There is no STL analogue
     *
     *  The method is added on request from ATLAS
     *  (see Savannah report #21395 and #21394)
     *
     *  @code
     *
     *  GaudiUtils::VectorMap<K,V> m = ... ;
     *
     *  K key  = ... ;
     *  V val1 = ... ;
     *  V val2 = ... ;
     *
     *  std::cout << "Value " << m[key] << std::endl ; // should be: V()
     *  m.update ( key , val1 ) ;
     *  std::cout << "Value " << m[key] << std::endl ; // should be: val1
     *  m.update ( key , val2 ) ;
     *  std::cout << "Value " << m[key] << std::endl ; // should be: val2
     *
     *  @endcode
     *
     *  @param key key value
     *  @param mapped mapped value
     *  @return true if the existing value has been replaced
     */
    bool update
    ( const key_type&    key    ,
      const mapped_type& mapped )
    {
      _iterator result = lower_bound ( key ) ;
      if ( end() == result || compare ( key , result -> first ) )
      {
        result = m_vct.insert ( result , value_type(key,mapped) ) ;
        return false ;
      }
      else { result->second = mapped ; }
      //
      return true ;
    }
    // ========================================================================
    /** forced insertion of the key/mapped pair
     *  The method acts like "insert" but it *DOES*
     *  overwrite the mapped value.
     *
     *  @attention There is no STL analogue
     *
     *  The method is added on request from ATLAS
     *  (see Savannah report #21395 and #21394)
     *
     *  @code
     *
     *  GaudiUtils::VectorMap<K,V> m = ... ;
     *
     *  K key  = ... ;
     *  V val1 = ... ;
     *  V val2 = ... ;
     *
     *  std::cout << "Value " << m[key] << std::endl ; // should be: V()
     *  m.update ( std::make_pair ( key , val1 ) ) ;
     *  std::cout << "Value " << m[key] << std::endl ; // should be: val1
     *  m.update ( std::make_pair ( key , val2 ) ) ;
     *  std::cout << "Value " << m[key] << std::endl ; // should be: val2
     *
     *  @endcode
     *
     *  @param  val a pair of (key,value)
     *  @return true if the existing value has been replaced
     */
    bool update ( const value_type& val )
    { return update ( val.first , val.second ) ; }
    // ========================================================================
    /** access to element by key (const version)
     *  there is no container increment for missing keys
     *
     *  @attention The behavior different from std::map,
     *             it is similar to GaudiUtils::Map
     *
     *  The method is added on request from ATLAS
     *  (see Savannah report #21395 and #21394)
     *  For typical usage of this class in LHCb context
     *  as "ExtraInfo" field I would like to recommend
     *  to AVOID this method
     *
     *  @code
     *
     *   GaudiUtils::VectorMap<K,V> m = ... ;
     *
     *   // OK:
     *   K key = ... ;
     *   std::cout << " Value: " << m(key) << std::end ; // it is OK!
     *
     *   // ERROR:
     *   V value = ... ;
     *   m(key) = value ;                                // ERROR!
     *
     *   @endcode
     *
     *  @see GaudiUtils::Map
     *  @param key key value
     *  @return mapped value for existing key and the
     *                 default value for non-existing key
     */
    const mapped_type& operator() ( const key_type& key ) const
    {
      static const mapped_type s_default = mapped_type() ;
      iterator res = find ( key ) ;
      if ( end() == res ) { return s_default ; }
      return res->second ;
    }
    // ========================================================================
    /** access to element by key (const version)
     *  there is no container increment for missing keys
     *
     *  @attention The behavior different from std::map,
     *             it is similar to GaudiUtils::Map
     *
     *  The method is added on request from ATLAS
     *  (see Savannah report #21395 and #21394)
     *  For typical usage of this class in LHCb context
     *  as "ExtraInfo" field I would like to recommend
     *  to AVOID this method
     *
     *  @code
     *
     *   GaudiUtils::VectorMap<K,V> m = ... ;
     *
     *   // OK:
     *   K key = ... ;
     *   std::cout << " Value: " << m[key] << std::end ; // it is OK!
     *
     *   // ERROR:
     *   V value = ... ;
     *   m[key] = value ;                                // ERROR!
     *
     *   @endcode
     *
     *  @see GaudiUtils::Map
     *  @param key key value
     *  @return mapped value
     */
    const mapped_type& operator[] ( const key_type& key ) const
    { return (*this)( key ) ; }
    // ========================================================================
    /** checked access to elements by key
     *  throw std::out_of_range exception for non-existing keys
     *
     *  @code
     *
     *   GaudiUtils::VectorMap<K,V> m = ... ;
     *
     *   // OK:
     *   K key = ... ;
     *   std::cout << " Value: " << m.at(key) << std::end ; // it is OK!
     *
     *  @endcode
     *
     *  @exception std::out_of_range for non-existing keys
     *  @param key key value
     *  @return mapped value
     */
    const mapped_type& at ( const key_type& key ) const
    {
      iterator res = find ( key ) ;
      if ( end() == res ) { this->throw_out_of_range_exception () ; }
      return res->second ;
    }
    // ========================================================================
  public:
    // ========================================================================
    // Constructors, destructors, etc.
    // ========================================================================
    /** default constructor from the the allocator
     *  @param cmp comparison criteria for the key
     *  @param alloc allocator to be used
     */
    VectorMap ( const allocator_type& alloc = allocator_type () )
      : m_vct ( alloc )
    {}
    // ========================================================================
    /** copy constructor
     *  @param right object to be copied
     */
    VectorMap ( const VectorMap& right )
      : Gaudi::Utils::MapBase(right), m_vct ( right.m_vct )
    {}
    // ========================================================================
    /** templated constructor from "convertible" sequence
     *  @param first 'begin'-iterator for the convertible sequence
     *  @param last  'end'-iterator for the convertible sequence
     *  @param cmp comparison criteria for the key
     *  @param alloc allocator to be used
     */
    template <class INPUT>
    VectorMap ( INPUT first ,
                INPUT last  ,
                const allocator_type& alloc = allocator_type () )
      : m_vct ( first , last , alloc )
    { std::sort ( m_vct.begin(), m_vct.end(), compare() ) ; }
    // ========================================================================
    /// destructor (non-virtual!)
    ~VectorMap() { clear() ; }                     // destructor (non-virtual!)
    // ========================================================================
    /* assignement operator
     * @param rigth object to be assigned
     * @return self
     */
    VectorMap& operator= ( const VectorMap& right )
    {
      if ( &right == this ) { return *this ; }
      m_vct = right.m_vct ;
      return *this ;
    }
    // ========================================================================
  public:
    // ========================================================================
    // The specific public accessors
    // ========================================================================
    /// get the comparison criteria itself
    const compare_type& compare     () const
    {
      static const  compare_type s_cmp = compare_type() ;
      return s_cmp ;
    }
    /// get the comparison criteria for keys
    const key_compare&  compare_key () const { return compare()  ; }
    /// printout to ostream - not implemented
    friend std::ostream& operator<<
      ( std::ostream& str , const VectorMap& /* obj */) { return str ; }
    // ========================================================================
  public:
    // ========================================================================
    /// merge two maps
    inline VectorMap& merge ( const VectorMap& right )
    {
      for ( const_iterator it = right.begin() ; right.end() != it ; ++it )
      { update ( it->first , it->second ) ; }
      //
      return *this ;
    }
    /// merge two maps
    template <class K1,class K2, class K3,class K4>
    inline VectorMap& merge ( const VectorMap<K1,K2,K3,K4>& right )
    {
      for ( typename VectorMap<K1,K2,K3,K4>::const_iterator it =
              right.begin() ; right.end() != it ; ++it )
      { update ( it->first , it->second ) ; }
      //
      return *this ;
    }
    // ========================================================================
  public:
    // ========================================================================
    /** useful method for python decoration:
     *  @param index (INPUT) the index
     *  @return the key at given index
     *  @exception std::out_of_range for invalid index
     */
    const key_type&    key_at   ( const size_t index ) const
    {
      if ( index  >= size() )
      { this->throw_out_of_range_exception () ; }
      const_iterator it = this->begin() ;
      std::advance ( it , index ) ;
      return it -> first ;
    }
    /** useful method for python decoration:
     *  @param index (INPUT) the index
     *  @return the value at given index
     *  @exception std::out_of_range for invalid index
     */
    const mapped_type& value_at ( const size_t index ) const
    {
      if ( index  >= size() )
      { this->throw_out_of_range_exception () ; }
      const_iterator it = this->begin() ;
      std::advance ( it , index ) ;
      return it -> second ;
    }
    // ========================================================================
  protected:
    // ========================================================================
    // Pure technical helper functions
    // ========================================================================
    /** compare the objects using the comaprison criteria
     *  @param obj the first object
     *  @param obj the second object
     *  @return result of (obj1,obj2) comparison
     */
    template <class TYPE1, class TYPE2>
    bool  compare ( const TYPE1& obj1 ,
                    const TYPE2& obj2 ) const
    {
      return compare() ( obj1 , obj2 )  ;
    }
    // ========================================================================
    /// 'lower-bound' - non-const version
    _iterator lower_bound ( const key_type& key )
    {
      return std::lower_bound
      ( m_vct.begin() , m_vct.end() , key , compare() ) ;
    }
    // ========================================================================
    /// the conversion from 'const' to 'non-const' iterator
    _iterator iter (  iterator p )
    {
      _iterator result = m_vct.begin() ;
      std::advance ( result , std::distance ( begin() , p ) ) ;
      return result ;
    }
    // ========================================================================
    /// the conversion from 'non-const' to 'const' iterator
    iterator  iter ( _iterator p )
    {
      iterator result ( begin() ) ;
      std::advance ( result , std::distance (  m_vct.begin() , p ) ) ;
      return result ;
    }
    // ========================================================================
  private:
    // ========================================================================
    /// the underlying sorted vector of (key,mapped) pairs
    _vector      m_vct ; // the underlying sorted vector of (key,mapped) pairs
    // ========================================================================
  };
  // ==========================================================================
} //                                               end of namespace GaudiUtils
// ============================================================================
namespace std
{
  // ==========================================================================
  /** the definition of specialized algorithm for swapping
   *  @param left  object to be swapped
   *  @param right object to be swapped
   */
  template
  < class KEY        ,
    class VALUE      ,
    class KEYCOMPARE ,
    class ALLOCATOR  >
  inline void swap
  ( GaudiUtils::VectorMap<KEY,VALUE,KEYCOMPARE,ALLOCATOR>& left  ,
    GaudiUtils::VectorMap<KEY,VALUE,KEYCOMPARE,ALLOCATOR>& right )
  { left.swap( right ) ; }
 // ===========================================================================
} //                                                       end of namespace std
// ============================================================================
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
     *  @see Gaudi::VectorMap
     *  @attention: this function is needed to use it as property
     *  @param result (OUTPUT) the parsing result
     *  @param input the input string
     *  @return status code
     */
    GAUDI_API StatusCode parse
    ( GaudiUtils::VectorMap<std::string, double>&  result ,
      const std::string& input  ) ;
    // ========================================================================
    /** parse the vector of keys from the string
     *  @see Gaudi::Parsers
     *  @see Gaudi::Parsers::parse
     *  @see Gaudi::VectorMap
     *  @see Gaudi::StringKey
     *  @attention: this function is needed to use it as property
     *  @param result (OUTPUT) the parsing result
     *  @param input the input string
     *  @return status code
     */
    GAUDI_API StatusCode parse
    ( GaudiUtils::VectorMap<Gaudi::StringKey, double>&  result ,
      const std::string&              input  ) ;
    // ========================================================================
  } //                                          end of namespace Gaudi::Parsers
  // ==========================================================================
} //                                                     end of namespace Gaudi


// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_MAPS_H
// ============================================================================
