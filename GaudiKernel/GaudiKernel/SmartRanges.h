#ifndef GAUDIKERNEL_SMARTRANGES_H
#define GAUDIKERNEL_SMARTRANGES_H 1
// ============================================================================
// STD&STL
// ============================================================================
#include <cassert>
#include <limits>
// ============================================================================
// Gaudikernel
// ============================================================================
// #include "GaudiKernel/Range.h"
#include "Range.h"
// ============================================================================
// boost
// ============================================================================
#include "boost/iterator/iterator_facade.hpp"
#include "boost/iterator/filter_iterator.hpp"
#include "boost/iterator/transform_iterator.hpp"
// ============================================================================
/** @file GaudiKernel/SmartRanges.h SmartRanges.h
 *  Collection of helper ranges/adaptors/algorithms  for smart
 *  and efficient manipulaitons with data
 *  - make_filter_range
 *  - make_transform_range
 *  - make_mask_range
 *  - make_index_range
 *  All these smart ranges can be chained/composed
 *  @author  Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2016-09-20
 */
// ============================================================================
namespace Gaudi
{
  // ==========================================================================
  namespace details
  {
    // ========================================================================
    /** "masked" iterator
     *  iterates ober "masked" intries in the cotainner/range
     *  - MASK can be something like boost::dynamic_bitset
     *  @author  Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2016-09-20
     */
    template <class ITERATOR, class MASK>
    class masked_iterator
      : public boost::iterator_facade<masked_iterator<ITERATOR,MASK>,
                                      typename std::iterator_traits<ITERATOR>::value_type,
                                      boost::forward_traversal_tag,
                                      typename std::iterator_traits<ITERATOR>::reference,
                                      typename std::iterator_traits<ITERATOR>::difference_type>
    {
    public:
      // ========================================================================
      // masked_iterator() = default ;
      masked_iterator ( const MASK& mask , ITERATOR b )
        : m_iterator ( b    )
        , m_mask     ( mask )
        , m_index    ( mask.find_first() )
      {
        if ( MASK::npos != m_index ) { std::advance ( m_iterator , m_index ) ; }
      }
      // ========================================================================
      masked_iterator ( MASK&& mask , ITERATOR b )
        : m_iterator ( b )
        , m_mask     ( mask )
        , m_index    ( mask.find_first() )
      {
        if ( MASK::npos != m_index ) { std::advance ( m_iterator , m_index ) ; }
      }
      // ========================================================================
      masked_iterator ()
        : m_iterator ()
        , m_mask     ()
        , m_index    ( MASK::npos )
      {}
      // ========================================================================
    private:
      // ========================================================================
      friend class boost::iterator_core_access ;
      void increment()
      {
        const size_t m = m_mask.find_next( m_index  ) ;
        if ( m != MASK::npos )  { std::advance ( m_iterator , m - m_index ) ; }
        m_index = m ;
      }
      // ========================================================================
      typename std::iterator_traits<ITERATOR>::reference
      dereference  () const { return *m_iterator ; }
      // ========================================================================
      bool equal( const masked_iterator& other ) const
      { return m_index == other.m_index ; }
      // ========================================================================
    private:
      // ========================================================================
      ITERATOR m_iterator ;
      MASK     m_mask     ;
      size_t   m_index    ;
      // ========================================================================
    };
    // ==========================================================================
    /** "index" iterator
     *  iterates ober "indiced" intries in the cotainer/range
     *  - INDICES can be something like std::vector<unsigned int>
     *  @author  Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2016-09-20
     */
    template <class ITERATOR, class INDICES>
    class index_iterator :
      public boost::iterator_facade<index_iterator<ITERATOR,INDICES>,
                                    typename std::iterator_traits<ITERATOR>::value_type,
                                    boost::forward_traversal_tag,
                                    typename std::iterator_traits<ITERATOR>::reference,
                                    typename std::iterator_traits<ITERATOR>::difference_type>
    {
    public:
      // ========================================================================
      index_iterator ( const INDICES& indices , ITERATOR b , ITERATOR e )
        : m_begin    ( b )
        , m_size     ( std::distance ( b , e ) )
        , m_indices  ( indices )
        , m_index    ( 0 )
      {}
      // ========================================================================
      index_iterator (  INDICES&& indices , ITERATOR b , ITERATOR e )
        : m_begin    ( b )
        , m_size     ( std::distance ( b , e ) )
        , m_indices  ( indices )
        , m_index    ( 0 )
      {}
      // ========================================================================
      index_iterator ()
        : m_begin    (   )
        , m_size     ( 0 )
        , m_indices  (   )
        , m_index    ( 0 )
      {}
      // ========================================================================
    private:
      // ========================================================================
      friend class boost::iterator_core_access ;
      void increment()
      {
        if ( m_index < m_indices.size() )
        {
          // 1. advance
          ++m_index ;
          // 2. advance further, if out of range
          if ( m_size <= m_indices[m_index] ) { increment () ; }
        }
      }
      typename std::iterator_traits<ITERATOR>::reference dereference  () const
      {
        ITERATOR it = m_begin ;
        if ( m_index < m_indices.size() )
        { it = std::next ( m_begin , m_indices[ m_index ] ) ; }
        return *it;
      }
      // ========================================================================
      bool equal( const index_iterator& other ) const
      {
        if  ( m_index == other.m_index && m_indices.size() == other.m_indices.size() ) { return true ; }
        return  // special stuff to catch "end"
          (       m_index ==       m_indices.size() ) &&
          ( other.m_index == other.m_indices.size() ) ;
      }
      // ========================================================================
    private:
      // ========================================================================
      ITERATOR    m_begin    ;
      std::size_t m_size     ;
      INDICES     m_indices  ;
      std::size_t m_index    ;
      // ======================================================================
    };
    // =======================================================================
  } // end of namespace Gaudi::details
  // =========================================================================
  /** create a filtering range with predicate
   *  @code
   *  // data container
   *  const std::vector<int> data = { 0,1,2,3,4,5,6,7,8,9,10 } ;
   *  // filter good elements
   *  auto good = Gaudi::make_filter_range ( [] (int i)->bool{ return x%2 } , data ) ;
   *  std::cout << "FILTER x%2 " ;
   *  std::copy ( good.begin(), good.end(), std::ostream_iterator<int>( std::cout ," " ) );
   *  std::cout << "\n" << std::endl;
   *  @endcode
   *  Filter ranges can be composed with other "smart" ranges.
   *  @attention Due to strange "feature" (gcc bug?) in the composition
   *  chain only one predicate in form of lambda-expression (or std::function
   *  from lambda expression) is alllowed. Number of simple/plain predicates
   *  in the composition chain can be arbitrary.
   *
   *  @see Gaudi::Range_
   *
   *  The result of <code>Gaudi::make_filter_range</code> does not contains
   *  the filtered elements and the function does not perform
   *  the looping over input container.
   *  The output contains only the recipe forfiltering during the subsequent looping.
   *  Actual filtering occurs during the traversal of the output range.
   *
   *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
   *  @date 2016-09-20
   */
  template <class PREDICATE,class CONTAINER>
  inline
  Gaudi::Range_<typename Gaudi::details::container<CONTAINER>::Container,
                boost::filter_iterator<PREDICATE,typename Gaudi::details::container<CONTAINER>::Iterator> >
  make_filter_range ( PREDICATE p , const CONTAINER& c )
  {
    typedef  boost::filter_iterator<PREDICATE,typename Gaudi::details::container<CONTAINER>::Iterator> ITER ;
    return { ITER( p , c.begin() ) , ITER ( p , c.end  () ) } ;
  }
  // ==========================================================================
  /** create "transform" range with functors
   *  @code
   *  std::vector<int> data = { 1,2,3,4,5,6,7,8 } ;
   *  auto good = Gaudi::make_transform_range ( [](int i)->double{ return 0.1*x } , data ) ;
   *  std::cout << "TRANSFORM x/10: " ;
   *  std::copy ( good.begin(), good.end(), std::ostream_iterator<int>( std::cout ," " ) );
   *  std::cout << "\n" << std::endl;
   *  @endcode
   *  Transform ranges can be composed with other "smart" ranges.
   *  @see Gaudi::Range_
   *
   *  @code
   *  // container of tracks
   *  CONTAINER tracks = ... ;
   *  // get "container" of transverse momenta
   *  auto pt = make_transform_range ( []( const Track* t ) -> double { return t->pt() ; } , tracks ) ;
   *  for  ( auto v : pt ) { histrogram.fill ( v ) ; }
   *  @endcode
   *
   *  The result of <code>Gaudi::make_transform_range</code> does not contains
   *  the transformed elements and the function does not perform
   *  the looping over input container.
   *  The output contains only the recipe for transforming during the subsequent looping.
   *  Actual transformation occurs during the traversal of the output range.
   *
   *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
   *  @date 2016-09-20
   */
  template <class FUNCTOR,class CONTAINER>
  inline
  Gaudi::Range_<typename Gaudi::details::container<CONTAINER>::Container,
                boost::transform_iterator<FUNCTOR,typename Gaudi::details::container<CONTAINER>::Iterator> >
  make_transform_range ( FUNCTOR f , const CONTAINER& c )
  {
    typedef boost::transform_iterator<FUNCTOR,typename Gaudi::details::container<CONTAINER>::Iterator> ITER ;
    return { ITER ( c.begin() , f ) , ITER ( c.end  () , f ) } ;
  }
  // ==========================================================================
  /** create "masking" range with mask
   *  @code
   *  using mask_t = boost::dynamic_bitset<unsigned_long> ;
   *  mask_t mask (100) ;
   *  mask.set(2,true)
   *  mask.set(3,true)
   *  std::vector<int> vct = { 1,2,3,4,5,6,7,8 } ;
   *  auto good = Gaudi::make_mask_range ( mask , vct ) ;
   *  std::cout << "MASK(2,3): " ;
   *  std::copy ( good.begin(), good.end(), std::ostream_iterator<int>( std::cout ," " ) );
   *  std::cout << "\n" << std::endl;
   *  @endcode
   *  Mask ranges can be composed with other "smart" ranges (but only the cases
   *  where mask-range is the first in the composition chain are well-defined)
   *  @see Gaudi::Range_
   *
   *  The result of <code>Gaudi::make_mask_range</code> does not contains
   *  the filtered elements and the function does not perform
   *  the looping over input container.
   *  The output contains only the recipe for filtering during the subsequent looping.
   *  Actual filtering occurs during the traversal of the output range.
   *
   *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
   *  @date 2016-09-20
   */
  template<class MASK, class CONTAINER>
  inline
  Gaudi::Range_<typename Gaudi::details::container<CONTAINER>::Container,
                Gaudi::details::masked_iterator<typename Gaudi::details::container<CONTAINER>::Iterator,MASK> >
  make_mask_range ( MASK m , const CONTAINER& c )
  {
    typedef Gaudi::details::masked_iterator<typename Gaudi::details::container<CONTAINER>::Iterator,MASK> ITER ;
    if ( m.size() > c.size() ) { m.resize( c.size() ) ; }
    return { ITER ( std::move(m) , c.begin() ) , ITER () } ;
  }
  // ============================================================================
  /** create "index" range with indices
   *  @code
   *  const std::vector<unsigned int> good_indices = { 3,3,3,4,5} ;
   *  std::vector<int> data = { 1,2,3,4,5,6,7,8 } ;
   *  auto good = Gaudi::make_index_range ( good_indices , data ) ;
   *  std::cout << "INDICES(3,3,3,4,5): " ;
   *  std::copy ( good.begin(), good.end(), std::ostream_iterator<int>( std::cout ," " ) );
   *  std::cout << "\n" << std::endl;
   *  @endcode
   *  - the repetitive elements also can appear in the index range
   *  - the invalid out-of-range indices are skept
   *  - if input container does not provide random access iterators,
   *    the iteration could be relatively slow
   *
   *  Indexed ranges can be composed with other "smart" ranges (but only the cases
   *  where indexed-range is the first in the composition chain are well-defined)
   *  @see Gaudi::Range_
   *
   *  The result of <code>Gaudi::make_mask_range</code> does not contains
   *  the filtered elements and the function does not perform
   *  the looping over input container.
   *  The output contains only the recipe for filtering during the subsequent looping.
   *  Actual filtering occurs during the traversal of the output range.
   *
   *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
   *  @date 2016-09-20
   */
  template<class INDICES, class CONTAINER>
  inline
  Gaudi::Range_<typename Gaudi::details::container<CONTAINER>::Container,
         Gaudi::details::index_iterator<typename Gaudi::details::container<CONTAINER>::Iterator,INDICES>>
  make_index_range ( INDICES m , const CONTAINER& c )
  {
    static_assert(  std::is_convertible<typename INDICES::value_type,std::size_t>::value  ,"index type is not convertible") ;
    static_assert(  std::numeric_limits<typename INDICES::value_type>::is_specialized     ,"index type is not specialized") ;
    static_assert( !std::numeric_limits<typename INDICES::value_type>::is_signed          ,"index type must be unsigned"  ) ;
    typedef Gaudi::details::index_iterator<typename Gaudi::details::container<CONTAINER>::Iterator,INDICES> ITER ;
    return { ITER ( std::move(m) , c.begin() , c.end() ) , ITER () } ;
  }
  // =========================================================================
}
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_SMARTRANGES_H
// ============================================================================
