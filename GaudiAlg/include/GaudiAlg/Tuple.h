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
#ifndef GAUDIALG_TUPLE_H
#define GAUDIALG_TUPLE_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>

// ============================================================================
/** @file Tuple.h
 *
 *  Header file for class : Tuple
 *
 *  @date 2002-10-30
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 */
// ============================================================================

// ============================================================================
/** @namespace Tuples
 *
 *  General namespace for Tuple properties
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2004-01-23
 */
// ============================================================================

#include "GaudiAlg/TupleObj.h"
#include "GaudiKernel/SmartIF.h"

namespace Tuples {

  template <class ITEM>
  class TupleItem;

  /** @class Tuple Tuple.h GaudiAlg/Tuple.h
   *
   *  @brief A simple wrapper class over standard
   *                        Gaudi NTuple::Tuple facility
   *
   *  The main advantages of local ntuples with respect to 'standard'
   *  Gaudi NTuples ( NTuple::Tuple ) is their "locality".
   *  For 'standard' ntuples one need
   * <ol>
   * <li> Define all ntuple columns/items as
   *      data members of the algorithm </li>
   * <li> Book the  <tt>NTuple::Tuple</tt> object using
   *      <tt>INTupleSvc</tt></li>
   * <li> Add all defined columns/items to the booked ntuple </li>
   * <li> Fill ntuple records
   * </ol>
   *  Usually the first step is done in the header file (separate file!)
   *  of the algorithm, the second and the third steps are done in
   *  <tt>initialize()</tt> method of the algorithm and
   *  the fourth step is done somewhere in <tt>execute()</tt> method of
   *  the same algorithm. Such approach requires to keep track of the
   *  tuple structure through different method and event through different
   *  files. And even minor modification of the structure of the ntuple
   *  will require the modification of at least 2 methods and 2 files.
   *
   *  The <tt>Tuples::Tuple</tt> wrapper over standard Gaudi
   *  <tt>NTuple::Tuple</tt> class solves all above listed problems with
   *  "non-local" nature of Gaudi <tt>NTuple::Tuple</tt> objects.
   *
   *  <tt>Tuples::Tuple</tt> object is booked and used 'locally'.
   *  One does not need to pre-book the ntuple or its columns/items
   *  somewhere in different compilation units or other methods different
   *  from the actual point of using the ntuple.
   *
   *  The simplest example of usage Tuple object:
   *
   *  @code
   *  Tuple tuple = nTuple( "some more or less unique tuple title ");
   *  for( Loop D0 = loop( "K- pi+", "D0" ) , D0 , ++D0 )
   *  {
   *     tuple -> column ( "mass" , M  ( D0 ) / GeV ) ;
   *     tuple -> column ( "pt"   , PT ( D0 ) / GeV ) ;
   *     tuple -> column ( "p"    , P  ( D0 ) / GeV ) ;
   *     tuple -> write  () ;
   *  }
   *  @endcode
   *
   *  One could fill some Tuple variables in one go
   *
   *  @code
   *  Tuple tuple = nTuple( "some more or less unique tuple title ");
   *  for( Loop D0 = loop( "K- pi+", "D0" ) , D0 , ++D0 )
   *  {
   *     tuple -> column ( "mass"      , M  ( D0 ) / GeV ) ;
   *     tuple -> fill   ( "pt , p "   , PT ( D0 ) / GeV , P(D0) / GeV ) ;
   *     tuple -> write  () ;
   *  }
   *  @endcode
   *
   *  Even ALL variables could be filled in one go:
   *
   *  @code
   *  Tuple tuple = nTuple( "some more or less unique tuple title ");
   *  for( Loop D0 = loop( "K- pi+", "D0" ) , D0 , ++D0 )
   *  {
   *     tuple -> fill   ( "mass pt , p ", M(D0)/GeV,PT(D0)/GeV,P(D0)/GeV ) ;
   *     tuple -> write  () ;
   *  }
   *  @endcode
   *
   *  All these techniques could be easily combined in arbitrary ways
   *
   *  @see GaudiTupleAlg
   *  @see TupleObj
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *  @date   2003-02-24
   */
  class GAUDI_API Tuple {
  public:
    /** standard constructor
     *  @param tuple pointer to "real" tuple object
     */

    Tuple( std::shared_ptr<TupleObj> tuple ) : m_tuple( std::move( tuple ) ) {}

    virtual ~Tuple() = default;

    /** get the pointer to the underlying object
     *  @return pointer to underlying TupleObj
     */
    TupleObj* operator->() const { return m_tuple.get(); }

    /// check the validity of the tuple object
    bool valid() const { return static_cast<bool>( m_tuple ); }

  protected:
    TupleObj* tuple() { return m_tuple.get(); }

  private:
    /// The (shared) tuple object
    std::shared_ptr<TupleObj> m_tuple;
  };

  /** @class TupleColumn
   *
   *  Helper class which allows to extend the functionality
   *  of Tuple with possibility to use your own representation
   *  of complex objects.
   *
   *  It allows to extend the functionality of
   *  Tuples::Tuple and Tuples::TupleObj classes
   *  for your own needs, according to your own taste
   *  and without touching the classes at all.
   *  Neither the extension or the functionality through
   *  inheritance nor the extension through aggregation
   *  is used. One use the trick with
   *  template specialization of streamer operators.
   *
   *  Assuming one need to add into private code the
   *  N-Tuple representation of e.g. MyClass class
   *
   *  @code
   *
   *  // 0) Class which needs N-Tuple representation
   *  class MyClass
   *    {
   *       ...
   *      double field1() const ;
   *      double field2() const ;
   *      long   field3() const ;
   *      bool   field4() const ;
   *    };
   *
   *  // 1) define specialization of operator with needed
   *  //    representation
   *  template <>
   *  inline Tuples::Tuple& operator<<
   *     ( Tuples::Tuple&                     tuple ,
   *      const Tuples::TupleColumn<MyClass>& item  )
   *   {
   *      // no action for invalid tuple
   *      if( !tuple.valid() ) { return tuple ;}
   *      tuple->column( item.name() + "field1" , item.value().field1() );
   *      tuple->column( item.name() + "field2" , item.value().field2() );
   *      tuple->column( item.name() + "field3" , item.value().field3() );
   *      tuple->column( item.name() + "field4" , item.value().field4() );
   *      return tuple ;
   *   }
   *
   * // 3) use the operator to 'stream' objects of type MyClass ito
   * //  N-Tuple:
   *
   *    Tuple tuple = ... ;
   *    MyClass a  = ...  ;
   *    tuple << Tuples::make_column( "A" , a ) ;
   *
   *    // operators can be chained:
   *    MyClass a1 = ...  ;
   *    MyClass a2 = ...  ;
   *    MyClass a3 = ...  ;
   *    tuple << Tuples::make_column( "A1" , a1 )
   *          << Tuples::make_column( "A2" , a2 )
   *          << Tuples::make_column( "A3" , a3 ) ;
   *
   *  @endcode
   *
   *  Alternatively one can use function Tuples::Column
   *
   *  @code
   *
   *    //
   *    MyClass a1 = ...  ;
   *    MyClass a2 = ...  ;
   *    MyClass a3 = ...  ;
   *    tuple << Tuples::Column( "A1" , a1 )
   *          << Tuples::Column( "A2" , a2 )
   *          << Tuples::Column( "A3" , a3 ) ;
   *
   *  @endcode
   *
   *
   *  Using this technique one can put 'any' object into NTuple
   *  and create the own representation. E.g. if the  'standard'
   *  representation of HepLorentzVector is not suitable one
   *  can create the alternative representation.
   *
   *  Also one can create own representations of complex classes, e.g.
   *  class MCParticle :
   *
   *  @code
   *
   *  /// 1 ) define template specialization
   *  template <>
   *  inline Tuples::Tuple& operator<<
   *     ( Tuples::Tuple&                               tuple ,
   *      const Tuples::TupleColumn<const MCParticle*>& item  )
   *   {
   *     if( !tuple.valid() ) { return tuple ;}
   *     const MCParticle* mcp = item.value() ;
   *     tuple->column( item.name() + "Mom"   , mcp->momentum()          ) ;
   *     tuple->column( item.name() + "PID"   , mcp->particleID().pid()  ) ;
   *     tuple->column( item.name() + "hasVX" , 0 != mcp->originVertex() ) ;
   *   };
   *
   *  /// 2) use the specialization to feed Tuple
   *
   *
   *    Tuple tuple = ... ;
   *    const MCParticle* mcp = ... ;
   *    tuple << Tuples::Column( "MCP" , mcp ) ;
   *
   *  @endcode
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   */
  template <class ITEM>
  class TupleColumn {
  public:
    TupleColumn( std::string name, ITEM value ) : m_name( std::move( name ) ), m_value( std::move( value ) ) {}

    /// Return the column name
    const std::string& name() const { return m_name; }
    /// Return the column value
    const ITEM& value() const { return m_value; }

  private:
    std::string m_name;  ///< The column name
    ITEM        m_value; ///< The column value
  };

  /** helper function to create 'on-the-fly' the
   *  helper object Tuples::TupleColumn
   */
  template <class ITEM>
  inline TupleColumn<ITEM> make_column( std::string name, const ITEM& item ) {
    return { std::move( name ), item };
  }

  /** helper function to create 'on-the-fly' the
   *  helper object Tuples::TupleColumn
   */
  template <class ITEM>
  inline TupleColumn<const ITEM*> make_column( std::string name, const ITEM* item ) {
    return { std::move( name ), item };
  }

  /** helper function to create 'on-the-fly' the
   *  helper object Tuples::TupleColumn
   */
  template <class ITEM>
  inline TupleColumn<ITEM*> make_column( std::string name, ITEM* item ) {
    return { std::move( name ), item };
  }

  template <class ITEM>
  inline TupleColumn<ITEM> Column( std::string name, const ITEM& item ) {
    return make_column( std::move( name ), item );
  }

  template <class ITEM>
  inline TupleColumn<const ITEM*> Column( std::string name, const ITEM* item ) {
    return make_column( std::move( name ), item );
  }

  template <class ITEM>
  inline TupleColumn<ITEM*> Column( std::string name, ITEM* item ) {
    return make_column( std::move( name ), item );
  }

} // namespace Tuples

// ============================================================================
/// helper operator to feed Tuple with the data, see Tuples::TupleColumn
// ============================================================================
template <class ITEM>
inline Tuples::Tuple& operator<<( Tuples::Tuple& tuple, const Tuples::TupleColumn<ITEM>& item ) {
  if ( !tuple.valid() ) return tuple; // no action for invalid tuple
  tuple->column( item.name(), item.value() )
      .orThrow( "Failed to fill the tuple", "operator<<( Tuples::Tuple&, ...)" )
      .ignore();
  return tuple;
}
// ============================================================================

// ============================================================================
// THe END
// ============================================================================
#endif // GAUDIALG_TUPLE_H
