// $Id: SharedObjectsContainer.h,v 1.2 2008/10/10 10:26:14 marcocle Exp $
// ============================================================================
#ifndef GAUDIKERNEL_SHAREDOBJECTSCONTAINER_H
#define GAUDIKERNEL_SHAREDOBJECTSCONTAINER_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <algorithm>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/ObjectContainerBase.h"
// ============================================================================
/** @class SharedObjectsContainer GaudiKernel/SharedObjectsContainer.h
 *
 *  Very simple class to represent the container of objects which are
 *  not ownered by the container. This concept seem to be very useful for
 *  LHCb HLT, DaVinci, tracking, alignments.
 *
 *  @warning the container is not persistable (on-purpose)
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date   2008-07-23
 */
template <class TYPE>
class SharedObjectsContainer : public ObjectContainerBase
{
public:
  // ==========================================================================
  /// the actual container type
  typedef std::vector<const TYPE*>   ConstVector ;
  /// various types (to make STL happy)
  typedef typename ConstVector::value_type                         value_type ;
  typedef typename ConstVector::size_type                           size_type ;
  typedef typename ConstVector::reference                           reference ;
  typedef typename ConstVector::const_reference               const_reference ;
  typedef typename ConstVector::iterator                             iterator ;
  typedef typename ConstVector::const_iterator                 const_iterator ;
  typedef typename ConstVector::reverse_iterator             reverse_iterator ;
  typedef typename ConstVector::const_reverse_iterator const_reverse_iterator ;
  // ==========================================================================
public:
  // ==========================================================================
  // the default constructor (creates the empty vector)
  SharedObjectsContainer ()
    : ObjectContainerBase(), m_data() {} ;
  // the constructor from the data
  SharedObjectsContainer ( const ConstVector& data )
    : ObjectContainerBase(), m_data(data) {}
  /** the templated constructor from the pair of iterators
   *  @param first 'begin'-iterator of the input sequence
   *  @param last  'last'-iterator of the input sequence
   */
  template <class DATA>
  SharedObjectsContainer(DATA first, DATA last)
    : ObjectContainerBase(), m_data(first, last) {}
  /** the templated constructor from the pair of iterators and the predicate.
   *
   *  Only the elements which satisfy the criteria goes into the container
   *  e.g. create the container only with "basic" particles:
   *
   *  @code
   *
   *   typedef SharedObjectsContainer<LHCb::Particle>  PARTICLES ;
   *
   *   // some sequence of someting, convertiblle to "const LHCb::Particle*":
   *   SEQUENCE source = ... ;
   *
   *   PARTICLES* particles = new PARTICLES
   *          ( source.begin() ,
   *            source.end  () ,
   *            std::mem_fun(&LHCb::Particle::isBasic) ) ;
   *
   *  @endcode
   *
   *  @param first 'begin'-iterator if the input sequence
   *  @param last  'last'-iterator of the input sequence
   *  @param cut   pre predicate
   */
  template <class DATA, class PREDICATE>
  SharedObjectsContainer(DATA first, DATA last, const PREDICATE& cut)
    : ObjectContainerBase(), m_data()
  {
    insert ( first , last , cut ) ;
  }
  /// destructor
  virtual ~SharedObjectsContainer() { m_data.clear() ; }
  // ==========================================================================
public:
  // ==========================================================================
  /// Retrieve the unique class ID (virtual)
  virtual const CLID& clID() const
  { return SharedObjectsContainer<TYPE>::classID(); }
  /// Retrieve the unuqie class ID (static)
  static const CLID& classID()
  {
    static const CLID s_clid =
      ( ( static_cast<CLID>  ( -1 ) << 16 )  // 16 used and 16 empty bits
        & !CLID_ObjectVector                 // not an ObjectVector
        & !CLID_ObjectList                   // not an ObjectList
        & !static_cast<CLID> ( 0x00030000 )  // not a  KeyedContainer/map
        & !static_cast<CLID> ( 0x00040000 ) )// not a  KeyedContainer/hashmap
      + TYPE::classID() ;                // the specific CLID from the contents
    //
    return s_clid;
  }
  // ==========================================================================
public:
  // ==========================================================================
  /// get the access to the underlying container (const)
  inline const ConstVector& data    () const { return m_data ; }
  /// cast to the underlying container
  operator const ConstVector& () const { return data () ; }
  // ==========================================================================
public:
  // ==========================================================================
  /// get the actual size of the container
  size_type size  () const { return m_data.size  () ; }
  /// empty container?
  bool      empty () const { return m_data.empty () ; }
  /** insert one object into the container
   *  @param object object to be added
   */
  void push_back ( const TYPE* object ) { m_data.push_back ( object ) ; }
  /** insert one object into the container
   *  @param object object to be added
   */
  void insert    ( const TYPE* object ) { m_data.push_back ( object ) ; }
  /** add the sequence of objects into the container
   *  @param first 'begin'-iterator for the sequnce
   *  @param last  'end'-iterator for the sequence
   */
  template <class DATA>
  void insert
  ( DATA first ,
    DATA last  ) { m_data.insert ( end() , first ,last ) ; }
  /** add the sequence of 'good'objects into the container
   *
   *  Only the objects which satisfy the predicate go to the container,
   *  e.g. put into container only particles which transverse momentum
   *  in excess of 1 * GeV:
   *  @code
   *
   *   namespace bl = boost::lambda ;
   *   typedef SharedObjectsContainer<LHCb::Particle>  PARTICLES ;
   *
   *   // some sequence of someting, convertible to "const LHCb::Particle*":
   *   SEQUENCE source = ... ;
   *
   *   PARTICLES* particles = ... ;
   *
   *   particles -> insert
   *         ( source.begin() ,
   *           source.end  () ,
   *           bl::bind(&LHCb::Particle::pt,bl::_1) > 1 * Gaudi::Units::GeV ) ;
   *
   *  @endcode
   *
   *  @param first 'begin'-iterator for the sequnce
   *  @param last  'end'-iterator for the sequence
   *  @param cut    the predicate to be applied
   */
  template <class DATA, class PREDICATE>
  void insert
  ( DATA             first ,
    DATA             last  ,
    const PREDICATE& cut   )
  {
    m_data.reserve ( m_data.size() + std::distance ( first , last ) ) ;
    for ( ; first != last ; ++first )
    { if ( cut ( *first ) ) { m_data.push_back ( *first ) ; } }
  }
  /** get from the container all objects which satisfy the certain criteria
   *
   *  E.g. get all particles with transverse momentum in excess of 1 * GeV
   *
   *  @code
   *
   *   namespace bl = boost::lambda ;
   *   typedef SharedObjectsContainer<LHCb::Particle>  PARTICLES ;
   *   typedef std::vector<const LHCb::Particle*>      TARGET    ;
   *
   *   PARTICLES* particles = ... ;
   *
   *   TARGET target ;
   *
   *   particles -> get
   *         ( bl::bind(&LHCb::Particle::pt,bl::_1) > 1 * Gaudi::Units::GeV ,
   *           std::back_inserter ( target ) ) ;
   *
   *  @endcode
   *
   *  Essentially this functionality is very useful due to
   *  missing "std::copy_if".
   *
   *  @param cut the predicate
   *  @param outptut the output iterator
   *  @return the current position of the output itrator (almost useless)
   */
  template <class OUTPUT, class PREDICATE>
  OUTPUT get ( const PREDICATE& cut    ,
               OUTPUT           output ) const
  {
    for ( const_iterator iobj = begin() ; end() != iobj ; ++iobj )
    { if ( cut ( *iobj ) ) { *output = *iobj ; ++output ; } }
    return output ;
  }
  /// erase the object by iterator
  void erase ( iterator i ) { m_data.erase ( i ) ; }
  /** erase the objects which satisfy the criteria
   *
   *  E.g. remove all particles with small transverse momentum
   *
   *  @code
   *
   *   namespace bl = boost::lambda ;
   *   typedef SharedObjectsContainer<LHCb::Particle>  PARTICLES ;
   *
   *   PARTICLES* particles = ... ;
   *   particles -> erase
   *         ( bl::bind(&LHCb::Particle::pt,bl::_1) < 1 * Gaudi::Units::GeV ) ;
   *
   *  @endcode
   *  @see std::remove_if
   *  @param cut predicate
   */
  template <class PREDICATE>
  void erase ( const PREDICATE& cut )
  { m_data.erase( std::remove_if (  begin() , end() , cut ) , end() ) ; }
  /** erase the first occurance of the certain element
   *
   *  To remove <b>all</b> occurances one can use:
   *  @code
   *
   *   typedef SharedObjectsContainer<LHCb::Particle>  PARTICLES ;
   *   PARTICLES* particles = ... ;
   *
   *   const LHCb::Particle* B = ... ;
   *
   *   // remove all occurances
   *   while ( particles -> erase ( B ) ) {} ;
   *
   *  @endcode
   *
   *  @param object the element to be removed
   *  @return true if the element is removed
   */
  bool erase ( const TYPE* object )
  {
    iterator i = std::find ( begin() , end() , object ) ;
    if ( end() == i ) { return false ; }
    m_data.erase ( i ) ;
    return true ;
  }
  // ==========================================================================
public:
  // ==========================================================================
  /// index access
  reference       operator[] ( size_type index )       { return m_data   [index] ; }
  /// index access (const-version)
  const_reference operator[] ( size_type index ) const { return m_data   [index] ; }
  /// 'functional'-access
  reference       operator() ( size_type index )       { return m_data   [index] ; }
  /// 'functional'-access  (const version)
  const_reference operator() ( size_type index ) const { return m_data   [index] ; }
  /// checked access
  reference       at         ( size_type index )       { return m_data.at(index) ; }
  /// checked access (const-version)
  const_reference at         ( size_type index ) const { return m_data.at(index) ; }
  // ==========================================================================
public:
  // ==========================================================================
  iterator               begin  ()       { return m_data .  begin () ; }
  iterator               end    ()       { return m_data .  end   () ; }
  const_iterator         begin  () const { return m_data .  begin () ; }
  const_iterator         end    () const { return m_data .  end   () ; }
  reverse_iterator       rbegin ()       { return m_data . rbegin () ; }
  reverse_iterator       rend   ()       { return m_data . rend   () ; }
  const_reverse_iterator rbegin () const { return m_data . rbegin () ; }
  const_reverse_iterator rend   () const { return m_data . rend   () ; }
  // ==========================================================================
public:
  // ==========================================================================
  /// the first element (only for non-empty vectors)
  reference       front ()       { return m_data . front () ; }
  /// the first element (only for non-empty vectors) (const-version)
  const_reference front () const { return m_data . front () ; }
  /// the last  element (only for non-empty vectors)
  reference       back  ()       { return m_data . back  () ; }
  /// the last  element (only for non-empty vectors) (const-version)
  const_reference back  () const { return m_data . back  () ; }
  // ==========================================================================
public:
  // ==========================================================================
  /// equal content with other container ?
  bool operator== ( const SharedObjectsContainer& right ) const
  { return &right == this || right.m_data == m_data ; }
  /// equal content with corresponding vector ?
  bool operator== ( const ConstVector&            right ) const
  { return m_data == right ; }
  /// comparisons with other container
  bool operator < ( const SharedObjectsContainer& right ) const
  { return m_data <  right.m_data ; }
  /// comparisons with corresponding vector
  bool operator < ( const ConstVector&            right ) const
  { return m_data <  right        ; }
  // ==========================================================================
public: // ObjectContainerBase methods:
  // ==========================================================================
  /** Distance of a given object from the beginning of its container
   *  @param object the object to be checked
   */
  virtual long index( const ContainedObject* object ) const
  {
    const_iterator _i = std::find ( begin() , end() , object ) ;
    return end() != _i ? ( _i - begin() ) : -1 ;                  // RETURN
  }
  /** Pointer to an object of a given distance
   *  @param index th eindex to be checked
   *  @return the object
   */
  virtual ContainedObject* containedObject ( long index ) const
  {
    if ( 0 > index || !(index < (long) size () ) ) { return 0 ; }    // RETURN
    const ContainedObject* co = m_data[index] ;
    return const_cast<ContainedObject*>( co ) ;
  }
  /// Number of objects in the container
  virtual size_type numberOfObjects() const { return m_data.size() ; }
  /** Virtual functions (forwards to the concrete container definitions)
   *  Add an object to the container. On success the object's index is
   *  returned.
   */
  virtual long add ( ContainedObject* object)
  {
    if ( 0 == object ) { return -1 ; }                           // RETURN
    TYPE* _obj = dynamic_cast<TYPE*> ( object ) ;
    if ( 0 == _obj   ) { return -1 ; }                           // RETURN
    const size_type pos = size() ;
    push_back ( _obj ) ;
    return pos ;
  }
  /** Release object from the container (the pointer will be removed
   *  from the container, but the object itself will remain alive).
   *  If the object was found it's index is returned.
   */
  virtual long remove ( ContainedObject* value )
  {
    iterator _i = std::find ( begin() , end() , value ) ;
    if ( end() == _i ) { return -1 ; }                          // RETURN
    const size_type pos = _i - begin() ;
    m_data.erase ( _i ) ;
    return pos ;                                                // RETURN
  }
  // ==========================================================================
private:
  // ==========================================================================
  // the actual data
  ConstVector m_data ; // the actual data
  // ==========================================================================
};
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_SHAREDOBJECTSCONTAINER_H
// ============================================================================
