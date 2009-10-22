// $Id: Range.h,v 1.2 2008/11/03 11:24:47 marcocle Exp $
// ============================================================================
#ifndef GAUDI_RANGE_H
#define GAUDI_RANGE_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <utility>
#include <vector>
#include <algorithm>
// ============================================================================
/** @file
 *
 *  This file has been imported from
 *  <a href="http://savannah.cern.ch/projects/loki">LoKi project</a>
 *  <a href="http://cern.ch/lhcb%2Dcomp/Analysis/Loki">
 *  "C++ ToolKit  for Smart and Friendly Physics Analysis"</a>
 *
 *  The package has been designed with the kind help from
 *  Galina PAKHLOVA and Sergey BARSUK.  Many bright ideas,
 *  contributions and advices from G.Raven, J.van Tilburg,
 *  A.Golutvin, P.Koppenburg have been used in the design.
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2001-01-23
 */
// ============================================================================
namespace Gaudi
{
  namespace details
  {
    // ========================================================================
    /** Helpful function to throw an "out-of-range exception" for class Range_
     *  @see GaudiException
     *  @param index invalid index
     *  @param size  range size
     */
    void rangeException
    ( const long   index ,
      const size_t size  ) ;
    // ========================================================================
  }
  // ==========================================================================
  /** @struct RangeBase_ Range.h GaudiUtils/Range.h
   *  helper class to simplify the dealing with ranges in Python
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date 2006-09-01
   */
  struct RangeBase_
  {
#ifdef _WIN32
    virtual ~RangeBase_() {}
#else
  protected :
    ~RangeBase_ (){}
#endif
  } ;
  // ==========================================================================
  /** @class Range_ Range.h GaudiUtils/Range.h
   *
   *  Useful class for representation of "sequence" of the objects
   *  through the range of valid iterators.
   *
   *  The range could be created over *ALL* container types which
   *  supports at least bidirectional iterators.
   *
   *  The minimum requirements from the container type:
   *    - support the concept of "CONTAINER::value_type"
   *    - support the concept of "CONTAINER::const_iterator"
   *    - support the concept of "CONTAINER::const_reference"
   *    - support the concept of "CONTAINER::const_reverse_iterator"
   *    - the iterator should be ok for "std::distance" and "std::advance"
   *    - support for "const_iterator         CONTAINER::begin  () const"
   *    - support for "const_iterator         CONTAINER::end    () const"
   *    - support for "const_reverse_iterator CONTAINER::rbegin () const"
   *    - support for "const_reverse_iterator CONTAINER::rend   () const"
   *
   *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
   *  @date   2002-07-12
   */
  template <class CONTAINER>
  class Range_ : public RangeBase_
  {
  public:
    // ========================================================================
    typedef std::pair<typename CONTAINER::const_iterator,
                      typename CONTAINER::const_iterator> Base ;
    // ========================================================================
  public:
    // ========================================================================
    /// type for actual contained iterator
    typedef CONTAINER                                  Container              ;
    typedef typename Container::value_type             value_type             ;
    typedef typename Container::const_iterator         iterator               ;
    typedef typename Container::const_iterator         const_iterator         ;
    typedef typename Container::const_reverse_iterator reverse_iterator       ;
    typedef typename Container::const_reverse_iterator const_reverse_iterator ;
    typedef typename Container::const_reference        reference              ;
    typedef typename Container::const_reference        const_reference        ;
    /// internal types
    typedef std::pair<iterator,iterator>               _Base                  ;
    typedef Range_<Container>                          _Self                  ;
    // ========================================================================
  public:
    // ========================================================================
    /// default constructor
    Range_() : m_base( iterator() , iterator() ) {};
    /** Constructor
     *  @param ibegin  iterator to begin of the sequence
     *  @param iend    iterator to end   of the sequence
     */
    Range_( iterator ibegin , iterator iend ) : m_base ( ibegin , iend ) {} ;
    /** constructor from the pair of iterators
     *  @param base pair of the iterators
     */
    Range_( const Base&      base ) : m_base( base ) {};
    /** constructor from the container
     *  @param cont  reference to the container
     */
    Range_( const Container& cont ) : m_base( cont.begin() , cont.end() ) {} ;
    /* constructor of empty range/sequence
     * @param ibegin  iterator to begin of empty sequence
     */
    Range_( iterator ibegin       ) : m_base( ibegin , ibegin ) {};
    /// destructor
    ~Range_(){};
    // ========================================================================
    /// empty sequence ?
    inline bool   empty () const { return m_base.second == m_base.first  ; }
    /// size of the sequence (number of elements)
    inline size_t size  () const
    { return std::distance ( m_base.first  , m_base.second  ) ; }
    /// access to begin of the sequence (const version )
    inline iterator begin () const { return m_base.first  ; }
    /// access to end   of the sequence (const version)
    inline iterator end   () const { return m_base.second ; }
    /// access to begin of the reversed sequence (const)
    inline reverse_iterator rbegin   () const { return reverse_iterator ( end   () ) ; }
    /// access to begin of the reversed sequence (const)
    inline reverse_iterator rend     () const { return reverse_iterator ( begin () ) ; }
    /// access for the first element (only for non-empty ranges!)
    inline const_reference front () const { return *( begin ()     ) ; }
    /// access for the back  element (only for non-empty ranges!)
    inline const_reference back  () const
    {
      const_iterator i = end() ;
      std::advance ( i , -1 ) ;
      return *i ;
    }
    // ========================================================================
    /// get a "slice" of a range, in Python style
    inline Range_ slice( long index1 , long index2 ) const
    {
      // trivial cases
      if ( empty() || index1 == index2 ) { return Range_() ; } // RETURN
      // adjust indices
      if ( index1 < 0      ) { index1 += size () ; }
      if ( index2 < 0      ) { index2 += size () ; }
      // check
      if ( index1 < 0      ) { return  Range_ () ; }            // RETURN
      if ( index2 < index1 ) { return  Range_ () ; }            // RETURN

      if ( index1 > (long) size () ) { return  Range_() ; }     // RETURN
      if ( index2 > (long) size () ) { index2  = size() ; }

      const_iterator i1 = begin()  ;
      std::advance ( i1 , index1 ) ;
      const_iterator i2 = begin()  ;
      std::advance ( i2 , index2 ) ;
      // construct the slice
      return Range_( i1 , i2 ) ;                                 // RETURN
    }
    // ========================================================================
    /** non-checked access to the elements by index
     *  (valid only for non-empty sequences)
     *  @param index the index of the lement to be accessed
     */
    inline const_reference operator () ( const size_t index ) const
    {
      const_iterator i = begin() ;
      std::advance ( i , index ) ;
      return *i ;
    }
    /** non-checked access to the elements by index
     *  (valid only for non-empty sequences)
     *  @param index the index of the lement to be accessed
     */
    inline const_reference operator [] ( const long index ) const
    { return (*this)( index ) ; }
    /** Checked access to the elements by index
     *  (valid for all sequences)
     *  @exception GaudiException for out-of-range access
     *  @param index the index of the element to be accessed
     */
    inline const_reference at          ( const long index ) const
    {
      if ( index < 0 || index >= (long) size () )
      { Gaudi::details::rangeException( index , size() ) ; }
      return (*this) ( index );
    }
    // ========================================================================
  public:
    // ========================================================================
    /// compare with another range
    bool operator< ( const Range_& right ) const
    {
      return std::lexicographical_compare
        ( begin () , end () , right.begin () , right.end () ) ;
    }
    /// compare with the base container
    bool operator< ( const Container& right ) const
    {
      return std::lexicographical_compare
        ( begin () , end () , right.begin () , right.end () ) ;
    }
    // ========================================================================
  public:
    // ========================================================================
    /// equality with another range
    bool operator==( const Range_& right ) const
    {
      if ( &right        == this    ) { return true  ; } // RETURN
      if ( right.size () != size () ) { return false ; } // RETURN
      return std::equal ( begin () , end () , right.begin() ) ;
    }
    /// equality with the base container
    bool operator==( const Container& right ) const
    {
      if ( right.size () != size () ) { return false ; } // RETURN
      return std::equal ( begin () , end () , right.begin() ) ;
    }
    // ========================================================================
  public:
    // ========================================================================
    /// empty sequence?
    bool operator!        () const { return  empty () ; }
    // ========================================================================
  public:
    // ========================================================================
    /// conversion operator to the std::pair
    operator const Base&      () const { return   base () ; }
    /// conversion operator to the std::pair
    inline   const Base& base () const { return m_base    ; }
    // ========================================================================
  private:
    // ========================================================================
    // the base itself
    Base m_base ;    ///< the base itself
    // ========================================================================
  }; // end of class Range_
  // ==========================================================================
  /** simple function to create the range from the arbitrary container
   *
   *  @code
   *
   *    const CONTAINER& cnt = ... ;
   *
   *    Range_<CONTAINER> r = range ( cnt ) ;
   *
   *  @endcode
   *
   *  The range could be created over *ALL* container types which
   *  supports at least the bidirectional iterators.
   *
   *  The minimum requirements from the container type:
   *    - support the concept of "CONTAINER::value_type"
   *    - support the concept of "CONTAINER::const_iterator"
   *    - support the concept of "CONTAINER::const_reference"
   *    - support the concept of "CONTAINER::const_reverse_iterator"
   *    - the iterator should be ok for "std::distance" and "std::advance"
   *    - support for "const_iterator         CONTAINER::begin  () const"
   *    - support for "const_iterator         CONTAINER::end    () const"
   *    - support for "const_reverse_iterator CONTAINER::rbegin () const"
   *    - support for "const_reverse_iterator CONTAINER::rend   () const"
   *
   *  @author Vanya BELYAEV ibelyaev@physics.syre.edu
   *  @date 2007-11-29
   */
  template <class CONTAINER>
  inline
  Range_<CONTAINER>
  range ( const CONTAINER& cnt )
  { return Range_<CONTAINER>( cnt.begin() , cnt.end() ) ; }
  // ==========================================================================
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDI_RANGE_H
// ============================================================================
