// $Id: Map.h,v 1.6 2007/12/19 15:42:56 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.6 $
// ============================================================================
#ifndef GAUDIKERNEL_MAP_H
#define GAUDIKERNEL_MAP_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL 
// ============================================================================
#include <map>
// ============================================================================


namespace GaudiUtils 
{
  /** @class Map Map.h GaudiKernel/Map.h
   *
   * Extension of the STL map. 
   *  Provides const accessors and can be extended by inheritance.
   * @see std::map
   * Note: in order to make it possible to use either a map or a hash_map, 
   *  reverse iterators are not defined.
   *
   * @author Marco Clemencic
   * @date 2005-10-06
   */
  template <typename K, typename T, typename M = std::map<K,T> >
  class Map 
  {
  public:
    
    // ---- types
    typedef M                      map_type;
    typedef K                      key_type;
    typedef T                      mapped_type;
    
    // -- from unary_function
    typedef K                      argument_type;
    typedef T                      result_type;
    typedef std::pair<const K,T>   value_type;
    
    typedef typename map_type::size_type size_type;
  
    typedef typename map_type::iterator iterator;
    typedef typename map_type::const_iterator const_iterator;
    // typedef typename map_type::reverse_iterator reverse_iterator;
    // typedef typename map_type::const_reverse_iterator const_reverse_iterator;
    
  protected:
    
    map_type m_map;
    static const result_type s_null_value;
    
  public:
    // ---- Constructors
    
    /// Standard constructor
    Map(): m_map() {}
    
    /// Constructor from a standard map
    Map(const map_type& other): m_map(other) {}
    
    // /// Copy Constructor
    // Map(const Map& other): m_map(other.m_map) {}
    /// Construct from a subset.
    template <typename In>
    Map(In first, In last): m_map(first,last) {}
    
    /// Virtual destructor. You can inherit from this map type.
    virtual ~Map() {}
    
    // ---- std::map interface
    
    //    -- iterators
    
    inline iterator begin() { return m_map.begin(); }
    inline iterator end()   { return m_map.end(); }
    
    inline const_iterator begin() const { return m_map.begin(); }
    inline const_iterator end()   const { return m_map.end(); }
  
    // inline reverse_iterator rbegin() { return m_map.rbegin(); }
    // inline reverse_iterator rend()   { return m_map.rend(); }
  
    // inline const_reverse_iterator rbegin() const { return m_map.rbegin(); }
    // inline const_reverse_iterator rend()   const { return m_map.rend(); }
  
    //    -- subscription
  
    inline mapped_type &operator[] (const key_type &key) { return m_map[key]; }
  
    //    -- map operations
  
    inline iterator find(const key_type &key) { return m_map.find(key); }
    inline const_iterator find(const key_type &key) const { return m_map.find(key); }
  
    inline size_type count(const key_type &key) const { return m_map.count(key); }
  
    inline iterator lower_bound(const key_type &key) { return m_map.lower_bound(key); }
    inline const_iterator lower_bound(const key_type &key) const { return m_map.lower_bound(key); }
    inline iterator upper_bound(const key_type &key) { return m_map.upper_bound(key); }  
    inline const_iterator upper_bound(const key_type &key) const { return m_map.upper_bound(key); }
  
    inline std::pair<iterator,iterator> equal_range(const key_type &key)
    { return m_map.equal_range(key); }
    inline std::pair<const_iterator,const_iterator> equal_range(const key_type &key) const
    { return m_map.equal_range(key); }
  
    //    -- list operations
  
    inline std::pair<iterator,bool> insert(const value_type &val) { return m_map.insert(val); }
    inline iterator insert(iterator pos, const value_type &val) { return m_map.insert(pos,val); }
    template <typename In>
    inline void insert(In first, In last) { m_map.insert(first,last); }
  
    inline void erase(iterator pos) { m_map.erase(pos); }
    inline size_type erase(const key_type &key) { return m_map.erase(key); }
    inline void erase(iterator first, iterator last) { m_map.erase(first,last); }
    inline void clear() { m_map.clear(); }
  
    //    -- container operations
  
    inline size_type size() const { return m_map.size(); }
    inline size_type max_size() const { return m_map.max_size(); }
    inline bool empty() const { return size() == 0; }
    inline void swap(map_type& other) { m_map.swap(other); }
  
    // ---- extra functionalities
    
    /** Allow to use Map as an unary function.
     *  There is no automatic extension of the map for missing keys!
     *
     *  @attention The behaviour is different from std::map 
     *
     *  @code 
     *
     *   const GaudiUtils::Map<KEY,VALUE> m = ... ;
     *  
     *   // OK: 
     *   KEY key = ... ;
     *   std::cout << " Value: " << m(key) << std::end ; // it is OK! 
     * 
     *   // ERROR:
     *   VALUE value = ... ;
     *   m(key) = value ;                                // ERROR!
     *
     *   @endcode 
     *   @param  key the key 
     *   @return the mapped value(const reference!) for the existing key, 
     *           and the default value overwise 
     */
    inline const result_type &operator() ( const argument_type &key ) const
    {
      // static const result_type s_null_value;
      const_iterator it = m_map.find(key);
      if ( it != m_map.end() ) { return it->second ; }   
      // return the default value 
      return s_null_value;  ///< return the default value 
    }
    
    /** Access elements of a const Map.
     *  There is no automatic extension of the map for missing keys! 
     *
     *  @attention The behaviour is different from std::map 
     *
     *  @code 
     *
     *   const GaudiUtils::Map<KEY,VALUE> m = ... ;
     *  
     *   // OK: 
     *   KEY key = ... ;
     *   std::cout << " Value: " << m[key] << std::end ; // it is OK! 
     * 
     *   // ERROR:
     *   VALUE value = ... ;
     *   m[key] = value ;                                // ERROR!
     *
     *   @endcode 
     *   @param key the key 
     *   @return the mapped value(const reference!) for the existing key, 
     *           and the default value overwise 
     */
    inline const mapped_type &operator[] ( const key_type &key ) const
    { return (*this)(key); }
    
    /// Merge two maps.
    inline Map& merge ( const map_type& other ) 
    {
      for ( typename map_type::const_iterator it = other.begin() ; 
            other.end() != it ; ++it ) { (*this)[it->first] = it->second ; }
      return *this;
    }
    
    /// Allows to use the Map wherever an std::map is explicitly requested.
    inline operator       map_type &()       { return m_map ; }
    inline operator const map_type &() const { return m_map ; }
  };
  
  template <typename K, typename T, typename M>
  const typename Map<K,T,M>::result_type Map<K,T,M>::s_null_value = typename Map<K,T,M>::result_type();

} // GaudiUtils namespace

// ============================================================================
/// The END 
// ============================================================================
#endif // GAUDIKERNEL_MAP_H
// ============================================================================
