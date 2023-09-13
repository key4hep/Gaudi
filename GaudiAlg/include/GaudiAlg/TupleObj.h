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
// ============================================================================
#ifndef GAUDIALG_TUPLEOBJ_H
#define GAUDIALG_TUPLEOBJ_H 1
// ============================================================================
// Include files
// ============================================================================
// STD&STL
// ============================================================================
#include <array>
#include <cstddef>
#include <fmt/format.h>
#include <functional>
#include <limits>
#include <numeric>
#include <set>
#include <sstream>
#include <string>

// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/SerializeSTL.h"
#include "GaudiKernel/VectorMap.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/Maps.h"
#include "GaudiAlg/Tuples.h"
// ============================================================================
// ROOT
// ============================================================================
#include "Math/Point3D.h"
#include "Math/SMatrix.h"
#include "Math/SVector.h"
#include "Math/Vector3D.h"
#include "Math/Vector4D.h"
// ============================================================================
// forward declaration
// ============================================================================
// GaudiKernel
// ============================================================================
class IOpaqueAddress;
// ============================================================================
/** @file TupleObj.h
 *
 *  Header file for class TupleObj
 *
 *  @date 2004-01-23
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 */

// ============================================================================
/** @namespace Tuples
 *
 *  General namespace for Tuple properties
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2004-01-23
 */
namespace Tuples {
  namespace detail {
    template <typename T>
    struct to_ {
      template <typename Arg>
      T operator()( Arg&& i ) const {
        return T( std::forward<Arg>( i ) );
      }
    };
    constexpr to_<float> to_float{};

    template <typename Iterator>
    using const_ref_t = std::add_const_t<typename std::iterator_traits<Iterator>::reference>;
  } // namespace detail
  // ==========================================================================
  /** @enum Type
   *  the list of available types for ntuples
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *  @date   2004-01-23
   */
  enum Type {
    NTUPLE, // Analysis nTuple
    EVTCOL  // Event Collection
  };
  // ==========================================================================
  /** @enum ErrorCodes
   *
   *  Tuple error codes
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *  @date   2004-01-23
   */
  enum class ErrorCodes : StatusCode::code_t {
    InvalidTuple = 100,
    InvalidColumn,
    InvalidOperation,
    InvalidObject,
    InvalidItem,
    TruncateValue = 200
  };
} // namespace Tuples

STATUSCODE_ENUM_DECL( Tuples::ErrorCodes )

namespace Tuples {
  // ==========================================================================
  /** @class TupleObj TupleObj.h GaudiAlg/TupleObj.h
   *
   *  @brief A simple wrapper class over standard
   *                        Gaudi NTuple::Tuple facility
   *
   *  The design and implementation are imported from LoKi package
   *
   *  One should not use lass TupleObj directly.
   *  The special handler Tuples::Tuple should be used instead,
   *  which is simultaneously 'proxy' an d'smart pointer' for
   *  real (and heavy!) TupleObj class.
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
   *  The simplest example of usage Tuples::Tuple object:
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
   *  @code
   *  Tuple tuple = nTuple( "some more or less unique tuple title ");
   *  for( Loop D0 = loop( "K- pi+", "D0" ) , D0 , ++D0 )
   *  {
   *     tuple -> fill   ( "mass pt , p ", M(D0)/GeV,PT(D0)/GeV,P(D0)/GeV ) ;
   *     tuple -> write  () ;
   *  }
   *  @endcode
   *
   *  The 'array-like' columns are also supported ( see methods 'farray')
   *
   *  All these techniques could be easily combined in arbitrary ways
   *
   *  class TupleObj is an abstract class with 3 pure abstract functions
   *  Error and Warning , which need to be reimplemented
   *  in any 'concrete class.
   *   Helper classes TupleObjImp, ErrorHandler and functions
   *   createTupleObj and make_handler allows to
   *   create concrete objects 'on-flight'
   *
   *  @attention
   *    <c>long long</c> and <c>unsigned long long</c>
   *    types are not supported. One needs to convert the data
   *    into some other representation (e.g. as 2 separate fields,
   *    or perform the explicitly cast to <c>long</c>)
   *
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date   2004-01-23
   */
  class GAUDI_API TupleObj {
  public:
    // ========================================================================
    /// basics type for various items
    using Bool      = NTuple::Item<bool>;
    using Char      = NTuple::Item<char>;
    using UChar     = NTuple::Item<unsigned char>;
    using Short     = NTuple::Item<short>;
    using UShort    = NTuple::Item<unsigned short>;
    using Int       = NTuple::Item<int>;
    using UInt      = NTuple::Item<unsigned int>;
    using LongLong  = NTuple::Item<long long>;
    using ULongLong = NTuple::Item<unsigned long long>;
    using Float     = NTuple::Item<float>;
    using Double    = NTuple::Item<double>;
    using Address   = NTuple::Item<IOpaqueAddress*>;
    using FArray    = NTuple::Array<float>;
    using FMatrix   = NTuple::Matrix<float>;
    // ========================================================================
    // the actual type for variable size matrix indices
    typedef unsigned short MIndex;
    // ========================================================================
    // the map of all items
    typedef std::map<std::string, std::string, std::less<>> ItemMap;
    // ========================================================================
  protected:
    // ========================================================================
    /** Standard constructor
     *  @see   NTuple:Tuple
     *  @param name  name of the object
     *  @param tuple pointer to standard Gaudi NTuple::Tuple object
     *  @param clid  CLID_ColumnWiseTuple or CLID_RowWiseTuple
     *  @param type the type of the tuple
     */
    TupleObj( std::string name, NTuple::Tuple* tuple, const CLID& clid = CLID_ColumnWiseTuple,
              const Tuples::Type type = Tuples::NTUPLE );
    // ========================================================================
  public:
    // ========================================================================
    virtual ~TupleObj();
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If column does not exist, it will be automatically created
     *  and appended to the tuple
     *
     *  @code
     *  //
     *  const float mass = ... ;
     *  tuple->column( "m", mass );
     *  //
     *  @endcode
     *
     *  @param  name  the name of the column
     *  @param  value the value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, float value );
    // ========================================================================
  public:
    // ========================================================================
    /** Set the value for the selected tuple column
     *  If the column does not exist, it will be automatically created
     *  and appended to the tuple
     *
     *  @code
     *  //
     *  const double mass = ... ;
     *  tuple->column( "m", mass );
     *  //
     *  @endcode
     *  @warning the value is truncated to float
     *
     *  @param  name     the name of the column
     *  @param  value    the value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, double value );
    // ========================================================================
  public:
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  short number = ... ;
     *  tuple -> column ( "num" , number );
     *
     *  @endcode
     *
     *  @param name   the name of the column
     *  @param value  the value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, short value );
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  short number = ... ;
     *  tuple->column( "num", number );
     *
     *  @endcode
     *
     *  @param name   name of the column
     *  @param value  value of the variable
     *  @param minv   minimum value of the variable
     *  @param maxv   maximum value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, short value, short minv, short maxv );
    // ========================================================================
  public:
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  unsigned short number = ... ;
     *  tuple -> column ( "num" , number );
     *
     *  @endcode
     *
     *  @param name   the name of the column
     *  @param value  the value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, unsigned short value );
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  unsigned short number = ... ;
     *  tuple->column( "num", number );
     *
     *  @endcode
     *
     *  @param name   name of the column
     *  @param value  value of the variable
     *  @param minv   minimum value of the variable
     *  @param maxv   maximum value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, unsigned short value, unsigned short minv, unsigned short maxv );
    // ========================================================================
  public:
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  char number = ... ;
     *  tuple -> column ( "num" , number );
     *
     *  @endcode
     *
     *  @param name   the name of the column
     *  @param value  the value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, char value );
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  char number = ... ;
     *  tuple->column( "num", number );
     *
     *  @endcode
     *
     *  @param name   name of the column
     *  @param value  value of the variable
     *  @param minv   minimum value of the variable
     *  @param maxv   maximum value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, char value, char minv, char maxv );
    // ========================================================================
  public:
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  unsigned char number = ... ;
     *  tuple -> column ( "num" , number );
     *
     *  @endcode
     *
     *  @param name   the name of the column
     *  @param value  the value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, unsigned char value );
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  unsigned char number = ... ;
     *  tuple->column( "num", number );
     *
     *  @endcode
     *
     *  @param name   name of the column
     *  @param value  value of the variable
     *  @param minv   minimum value of the variable
     *  @param maxv   maximum value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, unsigned char value, unsigned char minv, unsigned char maxv );
    // ========================================================================
  public:
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  int number = ... ;
     *  tuple->column("num", number );
     *
     *  @endcode
     *
     *  @param name   name of the column
     *  @param value  value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, int value );
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  int number = ... ;
     *  tuple->column( "num", number );
     *
     *  @endcode
     *
     *  @param name   name of the column
     *  @param value  value of the variable
     *  @param minv   minimum value of the variable
     *  @param maxv   maximum value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, int value, int minv, int maxv );
    // ========================================================================
  public:
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  unsigned int number = ... ;
     *  tuple->column("num", number );
     *
     *  @endcode
     *
     *  @param name   name of the column
     *  @param value  value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, unsigned int value );
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  unsigned int number = ... ;
     *  tuple->column("num", number );
     *
     *  @endcode
     *
     *  @param name   name of the column
     *  @param value  value of the variable
     *  @param minv   minimum value of the variable
     *  @param maxv   maximum value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, unsigned int value, unsigned int minv, unsigned int maxv );
    // ========================================================================
  public:
    // ========================================================================
    /** Set the value for the selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  long number = ... ;
     *  tuple -> column ( "num", number );
     *
     *  @endcode
     *  @warning the value could be truncated to int
     *
     *  @param name    the name of the column
     *  @param value   the value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, long value );
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  long number = ... ;
     *  tuple->column("num", number );
     *
     *  @endcode
     *
     *  @param name   name of the column
     *  @param value  value of the variable
     *  @param minv   minimum value of the variable
     *  @param maxv   maximum value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, long value, long minv, long maxv );
    // ========================================================================
  public:
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  unsigned long number = ... ;
     *  tuple -> column ( "num" , number );
     *
     *  @endcode
     *  @warning the value could be truncated to int
     *
     *  @param name   the name of the column
     *  @param value  the value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, unsigned long value );
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  unsigned long number = ... ;
     *  tuple->column( "num", number );
     *
     *  @endcode
     *
     *  @param name   name of the column
     *  @param value  value of the variable
     *  @param minv   minimum value of the variable
     *  @param maxv   maximum value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, unsigned long value, unsigned long minv, unsigned long maxv );
    // ========================================================================
  public:
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  long long number = ... ;
     *  tuple->column( "num", number );
     *
     *  @endcode
     *
     *  @param name   name of the column
     *  @param value  value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, long long value );
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  long long number = ... ;
     *  tuple->column( "num", number );
     *
     *  @endcode
     *
     *  @param name   name of the column
     *  @param value  value of the variable
     *  @param minv   minimum value of the variable
     *  @param maxv   maximum value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, long long value, long long minv, long long maxv );
    // ========================================================================
  public:
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  unsigned long long number = ... ;
     *  tuple->column( "num", number );
     *
     *  @endcode
     *
     *  @param name   name of the column
     *  @param value  value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, unsigned long long value );
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  unsigned long long number = ... ;
     *  tuple->column( "num", number );
     *
     *  @endcode
     *
     *  @param name   name of the column
     *  @param value  value of the variable
     *  @param minv   minimum value of the variable
     *  @param maxv   maximum value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, unsigned long long value, unsigned long long minv,
                       unsigned long long maxv );
    // ========================================================================
  public:
    // ========================================================================
    /** Set the value for the selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically created and appended to the tuple
     *
     *  @code
     *
     *  signed char number = ... ;
     *  tuple->column("num", number );
     *
     *  @endcode
     *
     *  @param name   the name of the column
     *  @param value  the value of tve variable
     *  @return status code
     */
    StatusCode column( std::string_view name, signed char value ) {
      return column( name, value, std::numeric_limits<signed char>::min(), std::numeric_limits<signed char>::max() );
    }
    // ========================================================================
  public:
    // ========================================================================
    /** Set the value for selected tuple column.
     *  If the column does not exist yet, it will be
     *  automatically create and appended to the tuple
     *
     *  @code
     *
     *  tuple->column( "empty" , v.empty()  );
     *
     *  @endcode
     *
     *  @param name   the name of the column
     *  @param value  the value of the variable
     *  @return status code
     */
    StatusCode column( std::string_view name, bool value );
    // ========================================================================
  public:
    // ========================================================================
    /** Put IOpaqueAddress in POOL-based NTuple.
     *  If the column does not exist,
     *  it will be automatically created
     *  and appended to the tuple.
     *
     *  @code
     *
     *  IOpaqueAddress* address  = ... ;
     *  tuple->column( "Address", address );
     *
     *  @endcode
     *  @warning It has sense only for Event tag collection N-Tuples
     *
     *  @param name name of the column
     *                       ("Address" is a recommended convention!)
     *  @param address IOpaqueAddress
     *  @return status code
     */
    StatusCode column( std::string_view name, IOpaqueAddress* address );
    // ========================================================================
    /** Put IOpaqueAddress in NTuple.
     *  If the column does not exist,
     *  it will be automatically created and appended to the tuple.
     *  The column name is set to be <c>"Address"</c>
     *
     *  @code
     *
     *  IOpaqueAddress* address  = ... ;
     *  tuple->column ( address  );
     *
     *  @endcode
     *  @warning It has sense only for Event tag collection N-Tuples
     *
     *  @param address IOpaqueAddress
     *  @return status code
     */
    StatusCode column( IOpaqueAddress* address );
    // ========================================================================
  public:
    // ========================================================================
    /** Set the values for several columns simultaneously, for the same object
     *  Non-existing columns will be automatically created
     *  and appended to the ntuple.
     *
     *  @code
     *
     *  Gaudi::XYZPoint p;
     *  tuple->columns( p, std::pair{ "X", &Gaudi::XYZPoint::X},
     *                     std::pair{ "Y", &Gaudi::XYZPoint::Y},
     *                     std::pair{ "Z", &Gaudi::XYZPoint::Z} );
     *
     *  @endcode
     *
     *  @warning the type of column is set (implicitly) by the type returned by the 'callable'
     *           for that column, i.e. in the above the return type of eg. `Gaudi::XYZPoint::X()`
     *
     *  @author Gerhard Raven
     */

    template <typename Value, typename... Args>
    StatusCode columns( Value&& value, Args&&... args ) {
      if ( sizeof...( Args ) == 0 ) return StatusCode::SUCCESS;
      std::initializer_list<StatusCode> scs{
          this->column( std::get<0>( args ), std::invoke( std::get<1>( args ), value ) )... };
      return std::accumulate( std::next( begin( scs ) ), end( scs ), *begin( scs ),
                              []( StatusCode sc, const StatusCode& i ) {
                                i.ignore();                     // make sure there are no unchecked StatusCodes...
                                return sc.isFailure() ? sc : i; // latch to the first non-success case
                              } );
    }
    // ========================================================================
  public:
    // ========================================================================
    /** Set the values for several columns simultaneously.
     *  Number of columns is arbitrary, but it should match
     *  the number of blank, or comma, or semi-column separated tags
     *  in <tt>format</tt> string.
     *  Non-existing columns will be automatically created
     *  and appended to the ntuple.
     *
     *  @code
     *
     *  double r1 , r2 , r3 , r4 , mass , length ;
     *  tuple->fill( "var1 var2, radius  rad4 mass  len" ,
     *                r1,  r2,   r3,     r4,  mass, length);
     *
     *  @endcode
     *
     *  @param format blank-separated list of variables,
     *          followed by variable number of arguments.
     *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
     *  @date 2002-10-30
     */
    template <typename Arg, typename... Args>
    StatusCode fill( std::string_view fmt, Arg arg, Args... args ) {
      constexpr auto separators = " ,;";
      // split into token, and remainder
      auto token = fmt.substr( 0, fmt.find_first_of( separators ) );
      fmt.remove_prefix( std::min( token.size() + 1, fmt.size() ) );

      if ( !token.empty() ) {
        // check the underlying tuple
        if ( invalid() ) return ErrorCodes::InvalidTuple;
        return column( token, arg ).andThen( [&] {
          if constexpr ( sizeof...( Args ) != 0 ) { return fill( fmt, args... ); }
          // no more args to deal with -- so check that there is nothing usefull left in fmt, and if so, call it a
          // success...
          if ( fmt.find_first_not_of( separators ) != std::string_view::npos )
            throw std::runtime_error{ "TupleObj::fill: bad format -- too few arguments for specified format" };
          return StatusCode{ StatusCode::SUCCESS };
        } );
      } else if ( !fmt.empty() ) {
        // got a separator at the front of fmt -- try again now that it is removed from fmt...
        return fill( fmt, arg, args... );
      } else {
        // empty token, and nothing left in fmt -- but we still where called with (at least) one argument...
        throw std::runtime_error{ "TupleObj::fill: bad format -- too many arguments for specified format" };
        return StatusCode::SUCCESS;
      }
    }

    // =======================================================================
  public:
    // =======================================================================
    /** Add an indexed array (of type float) to N-tuple.
     *  The method is not VERY efficient since it copies the data.
     *
     *  @code
     *
     *   std::vector<double> values  = ... ;
     *
     *   tuple->farray( "Values"        ,  // item name
     *                  values.begin () ,  // begin of sequence
     *                  values.end   () ,  // end of sequence
     *                  "Length"        ,  // name of "length" item
     *                  10000           ) ;
     *
     *  @endcode
     *
     *  The name of "length" item can be reused for several arrays.
     *  The last assignement "wins"
     *
     *   @code
     *
     *   std::vector<double> val1 = ... ;
     *   std::vector<double> val2 = ... ;
     *
     *   tuple->farray( "Val1"          ,   // item name
     *                   val1.begin  () ,   // begin of sequence
     *                   val1.end    () ,   // end of sequence
     *                  "Length"        ,   // name of "length" item
     *                  10000           ) ; // maximal length
     *
     *   tuple->farray( "Val2"          ,   // item name
     *                   val2.begin  () ,   // begin of sequence
     *                   val2.end    () ,   // end of sequence
     *                  "Length"        ,   // name of "length" item
     *                  10000           ) ; // maximal length
     *
     *  @endcode
     *
     *  Any sequence <tt>[first:last[</tt> of objects
     *  which can be converted to type <tt>float</tt> can
     *  be used as input data, e.g. <tt>std::vector<double></tt>,
     *     <tt>std::vector<float></tt>, plain C-array, or whatever else
     *
     *  @param name   name of N-tuple item
     *  @param first  begin of data sequence
     *  @param last   end of data sequence
     *  @param length name of "length" item
     *  @param maxv   maximal length of array
     */
    template <typename ITERATOR1, typename ITERATOR2>
    StatusCode farray( std::string_view name, ITERATOR1&& first, ITERATOR2&& last, std::string_view length,
                       size_t maxv ) {
      return farray( name, detail::to_float, std::forward<ITERATOR1>( first ), std::forward<ITERATOR2>( last ), length,
                     maxv );
    }
    // =======================================================================
    /** Add an indexed array (of type float) to N-tuple.
     *  it is just a small adaptor for the previous method
     *
     *  @code
     *
     *   std::vector<double> values  = ... ;
     *
     *   tuple->farray( "Values"        ,  // item name
     *                  values          ,  // sequence
     *                  "Length"        ,  // name of "length" item
     *                  10000           ) ;
     *
     *  @endcode
     *
     *  The name of "length" item can be reused for several arrays.
     *  The last assignment "wins"
     *
     *   @code
     *
     *   std::vector<double> val1 = ... ;
     *   std::vector<double> val2 = ... ;
     *
     *   tuple->farray( "Val1"          ,   // item name
     *                   val1           ,   // begin of sequence
     *                  "Length"        ,   // name of "length" item
     *                  10000           ) ; // maximal length
     *
     *   tuple->farray( "Val2"          ,   // item name
     *                   val2           ,   // begin of sequence
     *                  "Length"        ,   // name of "length" item
     *                  10000           ) ; // maximal length
     *
     *  @endcode
     *
     *  Any sequence which provides  <tt>begin()</tt> and
     *  <tt>end()</tt> methods can be used.
     *
     *  @param name   name of N-tuple item
     *  @param data   data sequence
     *  @param length name of "length" item
     *  @param maxv   maximal length of array
     */
    template <class DATA>
    StatusCode farray( std::string_view name, const DATA& data, std::string_view length, const size_t maxv ) {
      return farray( name, std::begin( data ), std::end( data ), length, maxv );
    }
    // =======================================================================
    /** Put an indexed array into LoKi-style N-Tuple
     *
     *  @code
     *
     *  std::vector<double> data = ... ;
     *
     *  Tuple tuple = ntuple( "My Ntuple" );
     *
     *  tuple->farray( "data"         ,   // data item name
     *                  sqrt          ,   // "function" to be applied
     *                  data.begin () ,   // begin of data sequence
     *                  data.end   () ,   // end of data sequence
     *                  "length"      ,   // name of "length" tuple item
     *                  10000         ) ; // maximal array length
     *
     *  @endcode
     *
     *  Since the method is templated, one can use arbitrary
     *  combinations of "sequences" and "functions", e.g. one can
     *  directly manipulate with complex objects.
     *  The only one thing is required - the result of
     *   <tt>FUNCTION(*DATA)</tt>  formal operation
     *  MUST be convertible to type <tt>float</tt>
     *
     *  @code
     *
     *  // some container of particles.
     *  ParticleVector particles = ... ;
     *
     *  Tuple tuple = ntuple( "My Ntuple" );
     *
     *  // put the transverse momentum of all particles into N-Tuple
     *  tuple->farray( "pt"                , // data item name
     *                  PT                 , // function object
     *                  particles.begin () , // begin of data sequence
     *                  particles.end   () , // end of data sequence
     *                  "num"              ,   // name of "length" tuple item
     *                  10000              ) ; // maximal array length
     *
     *
     *  // create the appropriate function object
     *  Fun fun =  Q / P ;
     *
     *  // put Q/P of all particles into N-Tuple
     *  tuple->farray( "qp"                , // data item name
     *                  fun                , // function object
     *                  particles.begin () , // begin of data sequence
     *                  particles.end   () , // end of data sequence
     *                  "num"              ,   // name of "length" tuple item
     *                  10000              ) ; // maximal array length
     *
     *
     *  @endcode
     *
     *  @param name     tuple item name
     *  @param function function to be applied
     *  @param first    begin of data sequence
     *  @param last     end of data sequence
     *  @param length   name of "length" tuple name
     *  @param maxv     maximal length of the array
     *  @return status code
     */
    template <class FUNCTION, class ITERATOR>
    StatusCode farray( std::string_view name, const FUNCTION& function, ITERATOR first, ITERATOR last,
                       std::string_view length, size_t maxv ) {
      if ( invalid() ) return ErrorCodes::InvalidTuple;
      if ( rowWise() ) return ErrorCodes::InvalidOperation;

      // adjust the length
      if ( std::distance( first, last ) > static_cast<std::ptrdiff_t>( maxv ) ) {
        Warning( fmt::format( "farray('{}'): array is overflow, skip extra entries", name ) )
            .ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
        last = std::next( first, maxv );
      }

      // get the length item
      Int* len = ints( length, 0, maxv );
      if ( !len ) return ErrorCodes::InvalidColumn;

      // adjust the length
      *len = std::distance( first, last );

      // get the array itself
      FArray* var = fArray( name, len );
      if ( !var ) return ErrorCodes::InvalidColumn;

      // fill the array
      std::transform( first, last, std::begin( *var ),
                      [&]( auto&& i ) { return std::invoke( function, std::forward<decltype( i )>( i ) ); } );

      return StatusCode::SUCCESS;
    }
    // =======================================================================
    /** Put arbitrary number of functions from one data array  into LoKi-style N-Tuple
     *  simultaneously (effective!)
     *
     *  @code
     *
     *  std::vector<double> data = ... ;
     *
     *  Tuple tuple = ntuple( "My Ntuple" );
     *
     *  tuple->farray( { { "sqrt", sqrt },     // name of 1st column, and function to use for it
     *                   { "sinus", sin } },   // name of the 2nd column, and corresponding function
     *                  data.begin () ,   // begin of data sequence
     *                  data.end   () ,   // end of data sequence
     *                  "length"      ,   // name of "length" tuple item
     *                  10000         ) ; // maximal array length
     *
     *  @endcode
     *
     *
     *  @param items    vector of pairs { name, callable }
     *  @param first    begin of data sequence
     *  @param last     end of data sequence
     *  @param length   name of "length" tuple name
     *  @param maxv     maximal length of the array
     *  @return status code
     */

    template <typename FunIterator, typename DataIterator>
    StatusCode farray_impl( FunIterator first_item, FunIterator last_item, DataIterator first, DataIterator last,
                            std::string_view length, size_t maxv ) {
      if ( invalid() ) return ErrorCodes::InvalidTuple;
      if ( rowWise() ) return ErrorCodes::InvalidOperation;

      // adjust the length
      if ( std::distance( first, last ) > static_cast<std::ptrdiff_t>( maxv ) ) {
        using GaudiUtils::details::ostream_joiner;
        std::ostringstream os;
        ostream_joiner( os, first_item, last_item, ",",
                        []( std::ostream& os, const auto& i ) -> decltype( auto ) { return os << i.first; } );
        Warning( "farray('" + os.str() + "'): array overflow, skipping extra entries" ).ignore();
        last = std::next( first, maxv );
      }

      // get the length item
      Int* len = ints( length, 0, maxv );
      if ( !len ) return ErrorCodes::InvalidColumn;

      // adjust the length
      *len = std::distance( first, last );

      // get the arrays themselves
      std::vector<FArray*> vars;
      vars.reserve( std::distance( first_item, last_item ) );
      std::transform( first_item, last_item, std::back_inserter( vars ),
                      [&]( const auto& item ) { return this->fArray( item.first, len ); } );
      if ( std::any_of( vars.begin(), vars.end(), []( const FArray* f ) { return !f; } ) ) {
        return ErrorCodes::InvalidColumn;
      }

      // fill the array
      for ( size_t index = 0; first != last; ++first, ++index ) {
        auto item = first_item;
        for ( auto& var : vars ) { ( *var )[index] = std::invoke( ( item++ )->second, *first ); }
      }

      return StatusCode::SUCCESS;
    }

    template <
        typename DataIterator, template <typename, typename...> class Container = std::initializer_list,
        typename NamedFunction = std::pair<std::string_view, std::function<float( detail::const_ref_t<DataIterator> )>>,
        typename               = std::enable_if_t<!std::is_convertible_v<Container<NamedFunction>, std::string_view>>>
    StatusCode farray( const Container<NamedFunction>& funs, DataIterator first, DataIterator last,
                       std::string_view length, size_t maxv ) {
      return farray_impl( funs.begin(), funs.end(), std::forward<DataIterator>( first ),
                          std::forward<DataIterator>( last ), length, maxv );
    }

    template <typename NamedFunctions, typename DataIterator,
              typename = std::enable_if_t<!std::is_convertible_v<NamedFunctions, std::string_view>>>
    StatusCode farray( const NamedFunctions& funs, DataIterator first, DataIterator last, std::string_view length,
                       size_t maxv ) {
      return farray_impl( funs.begin(), funs.end(), std::forward<DataIterator>( first ),
                          std::forward<DataIterator>( last ), length, maxv );
    }
    // =======================================================================
    /** Put two functions from one data array  into LoKi-style N-Tuple
     *  simultaneously (effective!)
     *
     *  @code
     *
     *  std::vector<double> data = ... ;
     *
     *  Tuple tuple = ntuple( "My Ntuple" );
     *
     *  tuple->farray( "square_root"  ,   // the first data item name
     *                  sqrt          ,   // "func1" to be used
     *                 "sinus"        ,   // the second data item name
     *                  sin           ,   // "func2" to be used
     *                  data.begin () ,   // begin of data sequence
     *                  data.end   () ,   // end of data sequence
     *                  "length"      ,   // name of "length" tuple item
     *                  10000         ) ; // maximal array length
     *
     *  @endcode
     *
     *
     *  @param name1    the first tuple item name
     *  @param func1    the first function to be applied
     *  @param name2    the second tuple item name
     *  @param func2    the second function to be applied
     *  @param first    begin of data sequence
     *  @param last     end of data sequence
     *  @param length   name of "length" tuple name
     *  @param maxv     maximal length of the array
     *  @return status code
     */
    template <class FUNC1, class FUNC2, class Iterator>
    StatusCode farray( std::string_view name1, const FUNC1& func1, std::string_view name2, const FUNC2& func2,
                       Iterator&& first, Iterator&& last, std::string_view length, size_t maxv ) {
      return farray( { { name1, std::cref( func1 ) }, { name2, std::cref( func2 ) } }, std::forward<Iterator>( first ),
                     std::forward<Iterator>( last ), length, maxv );
    }
    // =======================================================================
    /** Put three functions from one data array  into LoKi-style N-Tuple
     *  simultaneously (effective!)
     *
     *
     *  @code
     *
     *  std::vector<double> data = ... ;
     *
     *  Tuple tuple = ntuple( "My Ntuple" );
     *
     *  tuple->farray( "square_root"    ,   // the first data item name
     *                  sqrt          ,   // "func1" to be used
     *                 "sinus"        ,   // the second data item name
     *                  sin           ,   // "func2" to be used
     *                 "tan"          ,   // the third data item name
     *                  tan           ,   // "func3" to be used
     *                  data.begin () ,   // begin of data sequence
     *                  data.end   () ,   // end of data sequence
     *                  "length"      ,   // name of "length" tuple item
     *                  10000         ) ; // maximal array length
     *
     *  @endcode
     *
     *
     *  @param name1    the first tuple item name
     *  @param func1    the first function to be applied
     *  @param name2    the second tuple item name
     *  @param func2    the second function to be applied
     *  @param name3    the third tuple item name
     *  @param func3    the third function to be applied
     *  @param first    begin of data sequence
     *  @param last     end of data sequence
     *  @param length   name of "length" tuple name
     *  @param maxv     maximal length of the array
     *  @return status code
     */
    template <class FUNC1, class FUNC2, class FUNC3, class Iterator>
    StatusCode farray( std::string_view name1, const FUNC1& func1, std::string_view name2, const FUNC2& func2,
                       std::string_view name3, const FUNC3& func3, Iterator&& first, Iterator&& last,
                       std::string_view length, size_t maxv ) {
      return farray( { { name1, std::cref( func1 ) }, { name2, std::cref( func2 ) }, { name3, std::cref( func3 ) } },
                     std::forward<Iterator>( first ), std::forward<Iterator>( last ), length, maxv );
    }
    // =======================================================================
    /** Put four functions from one data array  into LoKi-style N-Tuple
     *  simultaneously (effective!)
     *
     *  @code
     *
     *  std::vector<double>   data = ... ;
     *
     *  Tuple tuple = ntuple( "My Ntuple" );
     *
     *  tuple->farray( "square_root"    ,   // the first data item name
     *                  sqrt          ,   // "func1" to be used
     *                 "sinus"        ,   // the second data item name
     *                  sin           ,   // "func2" to be used
     *                 "tan"          ,   // the third data item name
     *                  tan           ,   // "func3" to be used
     *                 "tanh"         ,   //
     *                  tanh          ,   //
     *                  data.begin () ,   // begin of data sequence
     *                  data.end   () ,   // end of data sequence
     *                  "length"      ,   // name of "length" tuple item
     *                  10000         ) ; // maximal array length
     *
     *  @endcode
     *
     *
     *  @param name1    the first tuple item name
     *  @param func1    the first function to be applied
     *  @param name2    the second tuple item name
     *  @param func2    the second function to be applied
     *  @param name3    the third tuple item name
     *  @param func3    the third function to be applied
     *  @param name4    the fourth tuple item name
     *  @param func4    the fourth function to be applied
     *  @param first    begin of data sequence
     *  @param last     end of data sequence
     *  @param length   name of "length" tuple name
     *  @param maxv     maximal length of the array
     *  @return status code
     */
    template <class FUNC1, class FUNC2, class FUNC3, class FUNC4, class Iterator>
    StatusCode farray( std::string_view name1, const FUNC1& func1, std::string_view name2, const FUNC2& func2,
                       std::string_view name3, const FUNC3& func3, std::string_view name4, const FUNC4& func4,
                       Iterator&& first, Iterator&& last, std::string_view length, size_t maxv ) {
      return farray( { { name1, std::cref( func1 ) },
                       { name2, std::cref( func2 ) },
                       { name3, std::cref( func3 ) },
                       { name4, std::cref( func4 ) } },
                     std::forward<Iterator>( first ), std::forward<Iterator>( last ), length, maxv );
    }
    // =======================================================================
  public:
    // =======================================================================
    /** Fill N-Tuple with data from variable-size matrix
     *
     *  "Matrix" could be of any type, which supports
     *   data[iRow][iCol] indexing, e.g.
     *    - std::vector<std::vector<TYPE> >
     *    - CLHEP::HepMatrix, etc...
     *
     *  @code
     *
     *   typedef std::vector<double> Row  ;
     *   typedef std::vector<Row>    Mtrx ;
     *   // number of columns (fixed!)
     *   const size_t numCols = 5 ;
     *   // maximal number of rows
     *   const size_t maxRows = 300 ;
     *   // number of rows (variable)
     *   size_t numRows =  .... ;
     *   ...
     *   tuple -> fMatrix ( "mtrx"        , // "column" name
     *                      mtrx          , // matrix
     *                      numRows       , // number of rows (variable!)
     *                      numCols       , // number of columns (fixed)
     *                      "Length"      , // name for "length" column
     *                      maxRows       ) ; // maximal number of columns
     *
     *  @endcode
     *
     *  @code
     *
     *   CLHEP::HepMatrix mtrx = ... ;
     *   ...
     *   tuple -> fMatrix ( "mtrx"         , // "column" name
     *                      mtrx           , // matrix
     *                      mtrx.num_row() , // number of rows (variable!)
     *                      mtrx.num_col() , // number of columns (fixed)
     *                      "Length"       , // name for "length" column
     *                      maxRows        ) ; // maximal number of columns
     *
     *  @endcode
     *
     *  @param name   entry name in N-Tuple
     *  @param data   matrix itself
     *  @param rows   number of rows of matrix (variable)
     *  @param cols   number of columns of matrix (fixed)
     *  @param length entry name in NTuple for number of matrix column
     *  @param maxv   maximal number of rows in matrix
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
     *  @date 2005-05-01
     */
    template <class MATRIX>
    StatusCode fmatrix( std::string_view name, const MATRIX& data, size_t rows, const MIndex& cols,
                        std::string_view length, size_t maxv ) {
      if ( invalid() ) return ErrorCodes::InvalidTuple;
      if ( rowWise() ) return ErrorCodes::InvalidOperation;

      // adjust the length
      if ( rows >= maxv ) {
        Warning( fmt::format( "fmatrix('{}'): matrix is overflow, skip extra items", name ) ).ignore();
        rows = ( 0 < maxv ) ? ( maxv - 1 ) : 0;
      }

      // get the length item
      Int* len = ints( length, 0, maxv );
      if ( !len ) return ErrorCodes::InvalidColumn;

      // adjust the length item
      *len = rows;

      // get the array itself
      FMatrix* var = fMatrix( name, len, cols );
      if ( !var ) return ErrorCodes::InvalidColumn;

      /// fill the matrix
      for ( size_t iCol = 0; iCol < cols; ++iCol ) {
        for ( MIndex iRow = 0; iRow < rows; ++iRow ) { ( *var )[iRow][iCol] = data[iRow][iCol]; }
      }

      return StatusCode::SUCCESS;
    }
    // =======================================================================
    /** Fill N-Tuple with data from variable-size matrix
     *
     *  "Matrix" could be of any type, which supports
     *   iteration from the first column to the last column
     *   and for each iterating column supports the indexing:
     *    (*first)[iCol]
     *
     *  @code
     *    typedef std::vector<double> Row  ;
     *    typedef std::vector<Row>    Mtrx ;
     *    // number of rows (fixed!)
     *    const size_t numRows = 5 ;
     *    // maximal number of columns
     *    const size_t maxCols = 300 ;
     *    // number of columns (variable)
     *    size_t numCols =  .... ;
     *    ...
     *    tuple -> fMatrix ( "mtrx"         , // entry name
     *                        mtrx.begin()  , // first row of matrix
     *                        mtrx.end  ()  , // last  row of matrix
     *                        numCols       , // number of columns (fixed!)
     *                        "Length"      , // name for "length" column
     *                        maxRows       ) ; // maximal number of rows
     *
     *  @endcode
     *
     *  @param name  entry name in N-Tuple
     *  @param first iterator for the first row of matrix
     *  @param last  iterator for the last  row of matrix
     *  @param cols  number of columns for matrix (fixed!)
     *  @param length entry name in NTuple for number of matrix column
     *  @param maxv maximal number of rows in matrix
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
     *  @date 2005-05-01
     */
    template <class DATA>
    StatusCode fmatrix( std::string_view name, DATA first, DATA last, const MIndex& cols, std::string_view length,
                        size_t maxv ) {
      if ( invalid() ) return ErrorCodes::InvalidTuple;
      if ( rowWise() ) return ErrorCodes::InvalidOperation;

      // adjust the length
      if ( first + maxv < last ) {
        Warning( fmt::format( "fmatrix('{}'): matrix is overflow, skip extra items", name ) ).ignore();
        last = first + maxv;
      }

      // get the length item
      Int* len = ints( length, 0, maxv );
      if ( !len ) return ErrorCodes::InvalidColumn;

      // adjust the length item
      *len = last - first;

      // get the array itself
      FMatrix* var = fMatrix( name, len, cols );
      if ( !var ) return ErrorCodes::InvalidColumn;

      /// fill the matrix
      size_t iRow = 0;
      for ( ; first != last; ++first ) {
        //
        for ( MIndex iCol = 0; iCol < cols; ++iCol ) { ( *var )[iRow][iCol] = ( *first )[iCol]; }
        //
        ++iRow;
      }

      return StatusCode::SUCCESS;
    }
    // =======================================================================
    /** fill N-Tuple with matrix of "direct-product" of
     *  "data-vector"     [first,last) and
     *  "function-vector" [funF, funL)
     *
     *  The elements of effective matrix are:
     *
     *   mtrx[iCol][iRow] = (*(funF+iRow))( *(first+iCol) )
     *
     *  @attention
     *  The length of data-vector is variable, while
     *  the length of "function" vector is fixed!
     *
     *  @code
     *
     *    typedef std::vector<double> Array ;
     *    Array array  = ... ;
     *
     *    typedef double (*fun)( double ) ;
     *    typedef std::vector<fun>   Funs ;
     *
     *    Funs funs ;
     *    funs.push_back( sin  ) ;
     *    funs.push_back( cos  ) ;
     *    funs.push_back( tan  ) ;
     *    funs.push_back( sinh ) ;
     *    funs.push_back( cosh ) ;
     *    funs.push_back( tanh ) ;
     *
     *    tuple->fmatrix ( "mtrx"         , // N-Tuple entry name
     *                     funs.begin  () , // begin of "function-vector"
     *                     funs.end    () , // end of "function-vector"
     *                     array.begin () , // begin of "data-vector"
     *                     array.end   () , // end of "data-vector"
     *                     "Length"       ,
     *                     100            ) ;
     *  @endcode
     *
     *  This method is very convenient e.g. for using within LoKi:
     *
     *  @code
     *
     *   typedef std::vector<Fun>  VctFun ;
     *
     *   // sequence of Particles
     *   Range particles = .... ;
     *
     *   // vector of functions:
     *  VctFun funs ;
     *  funs.push_back( E  / GeV ) ;
     *  funs.push_back( PX / GeV ) ;
     *  funs.push_back( PY / GeV ) ;
     *  funs.push_back( PZ / GeV ) ;
     *  funs.push_back( PT / GeV ) ;
     *  funs.push_back( M  / GeV ) ;
     *  funs.push_back( ID       ) ;
     *
     *  // fill N-Tuple with information about each particle
     *  tuple -> fmatrix ( "vars"             ,
     *                     funs.begin      () ,
     *                     funs.end        () ,
     *                     particles.begin () ,
     *                     particles.end   () ,
     *                     "nParts"           ,
     *                     200                ) ;
     *
     *  @endcode
     *
     *  @param name  entry name in N-Tuple
     *  @param funF  "begin"-iterator for vector of functions
     *  @param funL  "end"-iterator for vector of functions
     *  @param first "begin"-iterator for vector of data
     *  @param last  "end"-iterator for vector of data
     *  @param length entry name in NTuple for number of matrix column
     *  @param maxv maximal number of rows in matrix
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
     *  @date 2005-05-01
     */
    template <class FUN, class DATA>
    StatusCode fmatrix( std::string_view name, FUN funF, FUN funL, DATA first, DATA last, std::string_view length,
                        size_t maxv ) {
      if ( invalid() ) return ErrorCodes::InvalidTuple;
      if ( rowWise() ) return ErrorCodes::InvalidOperation;

      // adjust the length
      if ( std::distance( first, last ) > static_cast<std::ptrdiff_t>( maxv ) ) {
        Warning( fmt::format( "fmatrix('{}'): matrix is overflow, skip extra items", name ) ).ignore();
        last = std::next( first, maxv );
      }

      // get the length item
      Int* len = ints( length, 0, maxv );

      if ( !len ) return ErrorCodes::InvalidColumn;

      // adjust the length item
      *len = std::distance( first, last );

      // get the array itself
      auto     cols = std::distance( funF, funL );
      FMatrix* var  = fMatrix( name, len, cols );
      if ( !var ) return ErrorCodes::InvalidColumn;

      /// fill the matrix
      size_t iRow = 0;
      for ( ; first != last; ++first ) {
        //
        for ( FUN fun = funF; fun < funL; ++fun ) { ( *var )[iRow][fun - funF] = std::invoke( *fun, *first ); }
        //
        ++iRow;
      }

      return StatusCode::SUCCESS;
    }
    // =======================================================================
  public:
    // =======================================================================
    /** fill N-Tuple with fixed-size array
     *
     *  @code
     *
     *   SEQUENCE  data( 10 ) ;
     *   ...
     *   tuple -> array("data"         ,
     *                   data.begin () ,
     *                   data.end   () ) ;
     *
     *  @endcode
     *
     *  Sequence may be of any objects, implicitly
     *  convertible into "float"
     *
     *  @param name N-Tuple entry name
     *  @param first begin-iterator for data sequence
     *  @param last  end-iterator for data sequence
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
     *  @date 2005-05-01
     */
    template <class DATA>
    StatusCode array( std::string_view name, DATA first, DATA last )

    {
      if ( invalid() ) return ErrorCodes::InvalidTuple;
      if ( rowWise() ) return ErrorCodes::InvalidOperation;

      // get the length (fixed!)
      auto length = std::distance( first, last );

      // get the array itself
      FArray* var = fArray( name, length );
      if ( !var ) return ErrorCodes::InvalidColumn;

      /// fill the array
      std::copy( first, last, std::begin( *var ) );

      return StatusCode::SUCCESS;
    }
    // =======================================================================
    /** fill N-Tuple with fixed-size array
     *
     *  "ARRAY" must support indexing operations:
     *   e.g it coudl be of type:
     *   - std::vector<TYPE>
     *   - CLHEP::HepVector, ...
     *   - "TYPE"[n]
     *
     *   The content of array should be implicitly
     *   convertible to "float"
     *
     *  @code
     *
     *  CLHEP::HepVector vct1(10) ;
     *  ...
     *  tuple -> array ( "vct1" , vct1 , 10 ) ;
     *
     *  double vct2[40];
     *  ...
     *  tuple -> array ( "vct2" , vct2 , 40 ) ;
     *
     *  long   vct3[4];
     *  ...
     *  tuple -> array ( "vct3" , vct4 ,  4 ) ;
     *
     *  std::vector<long double> vct4(15) ;
     *  ...
     *  tuple -> array ( "vct4" , vct4 , 15 ) ;
     *
     *  @endcode
     *
     *  @param name    N-Tuple entry name
     *  @param data    data sequence
     *  @param length  data length (fixed!)
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
     *  @date 2005-05-01
     */
    template <class ARRAY>
    StatusCode array( std::string_view name, const ARRAY& data, const MIndex& length ) {
      using std::begin; // allow data to be eg. CLHEP::HepVector (which does not define HepVector::begin()!,
                        // in which case ADL prefers CLHEP::begin (yah! at least they (now) use a namespace)
                        // so one just to insure  double* CLHEP::begin(CLHEP::HepVector& v) { return &v[0]; }
                        // is visible when this function gets instantiated for CLHEP::HepVector...
      auto first = begin( data );
      return array( name, first, std::next( first, length ) );
    }
    // =======================================================================
    /** fill N-Tuple with fixed-size array
     *
     *  "ARRAY" is any sequence, which supports
     *   ARRAY::begin() and ARRAY::end() protocol, e.g.
     *
     *   - std::vector<TYPE>
     *   - ROOT::Math::SVector<double,15>
     *
     *  The content of array should be implicitly
     *  convertible to "float"
     *
     *  @code
     *
     *   typedef std::vector<double> Seq ;
     *   Seq data( 10 ) ;
     *   for ( int i = 0 ; i < 10 ; ++i )
     *    {
     *      data[i] = ... ;
     *    }
     *
     *  tuple -> array( "data" , data ) ;
     *
     *  @endcode
     *
     *  @param name N-Tupel entry name
     *  @param data  data sequence
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
     *  @date 2005-05-01
     */
    template <class ARRAY>
    StatusCode array( std::string_view name, const ARRAY& data ) {
      using std::begin;
      using std::end;
      return array( name, begin( data ), end( data ) );
    }
    // =======================================================================
  public:
    // =======================================================================
    /** fill N-Tuple with fixed-size matrix
     *
     *  "MATRIX" must support indexing operations:
     *    data[iRow][iCol]
     *
     *   e.g it could be of type:
     *   - std::vector<std::vector<TYPE> >
     *   - CLHEP::HepMatrix , CLHEP::GenMatrix, etc ...
     *   - "TYPE"[n][m]
     *
     *  The content of MATRIX should be implicitly convertible
     *  to "float"
     *
     *  @code
     *
     *   CLHEP::HepMatrix mtrx1(3,20) ;
     *   ...
     *   tuple -> matrix ( "m1"             ,
     *                      mtrx1           ,
     *                      mtrx1.num_row() ,
     *                      mtrx1.num_col() ) ;
     *
     *   typedef std::vector<double> Row  ;
     *   typedef std:vector<Row>     Mtrx ;
     *   Mtrx mtrx2( 3 , Row(10) ) ;
     *   ...
     *   tuple -> matrix ( "m2"   ,
     *                      mtrx2 ,
     *                      3     ,
     *                      10    ) ;
     *
     *   float mtrx3[3][10] ;
     *   ...
     *   tuple -> matrix ( "m3"   ,
     *                      mtrx3 ,
     *                      3     ,
     *                      10    ) ;
     *
     *  @endcode
     *
     *  @param name N-Tuple entry name
     *  @param data data source (matrix)
     *  @param cols number of columns
     *  @param rows number of rows
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
     *  @date 2005-05-01
     */
    template <class MATRIX>
    StatusCode matrix( std::string_view name, const MATRIX& data, const MIndex& rows, const MIndex& cols ) {
      if ( invalid() ) return ErrorCodes::InvalidTuple;
      if ( rowWise() ) return ErrorCodes::InvalidOperation;

      // get the matrix itself
      FMatrix* var = fMatrix( name, rows, cols );
      if ( !var ) return ErrorCodes::InvalidColumn;

      /// fill the matrix
      for ( size_t iCol = 0; iCol < cols; ++iCol ) {
        for ( size_t iRow = 0; iRow < rows; ++iRow ) { ( *var )[iRow][iCol] = data[iRow][iCol]; }
      };
      return StatusCode::SUCCESS;
    }
    // =======================================================================
  public:
    // =======================================================================
    /** Useful shortcut to put LorentzVector directly into N-Tuple:
     *
     *  @code
     *
     *  const LHCb::Particle* B = ...
     *
     *  Tuple tuple = nTuple("My N-Tuple") ;
     *
     *  // put 4-vector of B-candidate into N-tuple:
     *  tuple -> column ("B" , B->momentum() ) ;
     *
     *  @endcode
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-11-26
     */
    template <class TYPE>
    StatusCode column( std::string_view name, const ROOT::Math::LorentzVector<TYPE>& v ) {
      return columns( v, std::pair{ fmt::format( "{}E", name ), &ROOT::Math::LorentzVector<TYPE>::E },
                      std::pair{ fmt::format( "{}X", name ), &ROOT::Math::LorentzVector<TYPE>::Px },
                      std::pair{ fmt::format( "{}Y", name ), &ROOT::Math::LorentzVector<TYPE>::Py },
                      std::pair{ fmt::format( "{}Z", name ), &ROOT::Math::LorentzVector<TYPE>::Pz } );
    }
    // =======================================================================
    /** Useful shortcut to put 3D-Vector directly into N-Tuple:
     *
     *  @code
     *
     *  const LHCb::Vertex* V = ...
     *
     *  Tuple tuple = nTuple("My N-Tuple") ;
     *
     *  // put vertex position into N-tuple:
     *  tuple -> column ("B" , B->position() ) ;
     *
     *  @endcode
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-11-26
     */
    template <class TYPE, class TAG>
    StatusCode column( std::string_view name, const ROOT::Math::DisplacementVector3D<TYPE, TAG>& v ) {
      return this->columns( v, std::pair{ fmt::format( "{}X", name ), &ROOT::Math::DisplacementVector3D<TYPE, TAG>::X },
                            std::pair{ fmt::format( "{}Y", name ), &ROOT::Math::DisplacementVector3D<TYPE, TAG>::Y },
                            std::pair{ fmt::format( "{}Z", name ), &ROOT::Math::DisplacementVector3D<TYPE, TAG>::Z } );
    }
    // =======================================================================
    /** Useful shortcut to put 3D-Vector directly into N-Tuple:
     *
     *  @code
     *
     *  const LHCb::Vertex* V = ...
     *
     *  Tuple tuple = nTuple("My N-Tuple") ;
     *
     *  // put vertex position into N-tuple:
     *  tuple -> column ("B" , B->position() ) ;
     *
     *  @endcode
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-11-26
     */
    template <class TYPE, class TAG>
    StatusCode column( std::string_view name, const ROOT::Math::PositionVector3D<TYPE, TAG>& v ) {
      return this->columns( v, std::pair{ fmt::format( "{}X", name ), &ROOT::Math::PositionVector3D<TYPE, TAG>::X },
                            std::pair{ fmt::format( "{}Y", name ), &ROOT::Math::PositionVector3D<TYPE, TAG>::Y },
                            std::pair{ fmt::format( "{}Z", name ), &ROOT::Math::PositionVector3D<TYPE, TAG>::Z } );
    }
    // =======================================================================
    /** shortcut to put Smatrix into N-tuple:
     *
     *  @code
     *  @endcode
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-11-26
     */
    template <class TYPE, unsigned int D1, unsigned int D2, class REP>
    StatusCode matrix( std::string_view name, const ROOT::Math::SMatrix<TYPE, D1, D2, REP>& mtrx ) {
      if ( invalid() ) return ErrorCodes::InvalidTuple;
      if ( rowWise() ) return ErrorCodes::InvalidOperation;

      // get the matrix itself
      FMatrix* var = fMatrix( name, (MIndex)D1, (MIndex)D2 );
      if ( !var ) return ErrorCodes::InvalidColumn;

      /// fill the matrix
      for ( size_t iCol = 0; iCol < D2; ++iCol ) {
        for ( size_t iRow = 0; iRow < D1; ++iRow ) { ( *var )[iRow][iCol] = mtrx( iRow, iCol ); }
      };

      return StatusCode::SUCCESS;
    }
    // =======================================================================
    /** shortcut to put "ExtraInfo" fields of major
     *  into N-Tuple
     *
     *  @code
     *
     *  const LHCb::Particle* B = ...
     *
     *  Tuple tuple = nTuple("My N-Tuple") ;
     *
     *  // put the vector into N-Tuple:
     *  tuple -> fmatrix ( "Info" , B->extraInfo() , "nInfo" , 100 ) ;
     *
     *  @endcode
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-11-26
     */
    template <class KEY, class VALUE>
    StatusCode fmatrix( std::string_view name, const GaudiUtils::VectorMap<KEY, VALUE>& info, std::string_view length,
                        const size_t maxv = 100 ) {
      using Info                                                 = std::pair<KEY, VALUE>;
      static const std::array<float ( * )( const Info& ), 2> fns = {
          { []( const Info& i ) -> float { return i.first; }, []( const Info& i ) -> float { return i.second; } } };
      return fmatrix( name, begin( fns ), end( fns ), begin( info ), end( info ), length, maxv );
    }
    // =======================================================================
  public:
    // =======================================================================
    /** The function allows to add almost arbitrary object into N-tuple
     *  @attention it requires POOL persistency
     *  @param name column name
     *  @param obj  pointer to the object
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-04-08
     */
    template <class TYPE>
    StatusCode put( std::string_view name, const TYPE* obj );
    // =======================================================================
  public:
    // =======================================================================
    /** write a record to NTuple
     *  @return status code
     */
    StatusCode write();
    // =======================================================================
    /// get the name
    const std::string& name() const { return m_name; }
    // =======================================================================
    /** provide the access to underlying Gaudi N-tuple
     *  @return pointer to Gaudi N-tuple object
     */
    const NTuple::Tuple* tuple() const { return m_tuple; }
    // =======================================================================
    /** provide the access to underlying Gaudi N-tuple
     *  @return pointer to Gaudi N-tuple object
     */
    NTuple::Tuple* tuple() { return m_tuple; }
    // =======================================================================
    /// accessor to the N-Tuple CLID
    const CLID& clid() const { return m_clid; }
    // =======================================================================
    /// accessor to the N-Tuple type
    Tuples::Type type() const { return m_type; }
    // =======================================================================
    /// column wise NTuple ?
    bool columnWise() const { return CLID_ColumnWiseTuple == clid(); }
    // =======================================================================
    /// row wise NTuple ?
    bool rowWise() const { return CLID_RowWiseTuple == clid(); }
    // =======================================================================
    /// Event collection ?
    bool evtColType() const { return Tuples::EVTCOL == type(); }
    // =======================================================================
    /// valid pointer to tuple ?
    bool valid() const { return 0 != tuple(); }
    // =======================================================================
    /// invalid pointer to tuple ?
    bool invalid() const { return !valid(); }
    // =======================================================================
  public:
    // =======================================================================
    /** add the item name into the list of known items
     *  @param name the name of the item
     *  @param type the type of the item
     *  @return true if the name is indeed added
     */
    bool addItem( std::string name, std::string type ) {
      return m_items.emplace( std::move( name ), std::move( type ) ).second;
    }
    // =======================================================================
    /** check the uniqueness of the name
     *  @param name the name of the item
     *  @return true if the name is indeed unique
     */
    bool goodItem( std::string_view name ) const { return m_items.end() == m_items.find( name ); }
    // =======================================================================
    /// get the full list of booked items
    const ItemMap& items() const { return m_items; }
    // =======================================================================
  public:
    // =======================================================================
    virtual StatusCode Error( const std::string& msg, const StatusCode sc = StatusCode::FAILURE ) const = 0;
    // =======================================================================
    virtual StatusCode Warning( const std::string& msg, const StatusCode sc = StatusCode::FAILURE ) const = 0;
    // =======================================================================
  private:
    // =======================================================================
    /// get the column
    Int* ints( std::string_view name, int minv, int maxv );
    // =======================================================================
    /// get the column
    FArray* fArray( std::string_view name, Int* item );
    // =======================================================================
    /// get the column
    FArray* fArray( std::string_view name, const MIndex& rows );
    // =======================================================================
    /// get the column
    Address* addresses( std::string_view name );
    // =======================================================================
    /// get the column
    FMatrix* fMatrix( std::string_view name, Int* item, const MIndex& cols );
    // =======================================================================
    /// get the column
    FMatrix* fMatrix( std::string_view name, const MIndex& rows, const MIndex& cols );
    // =======================================================================
    /// delete the copy constructor and assignment
    TupleObj( const TupleObj& )            = delete;
    TupleObj& operator=( const TupleObj& ) = delete;
    // =======================================================================
  private:
    // =======================================================================
    /// name
    std::string m_name;
    // =======================================================================
    /// tuple itself
    NTuple::Tuple* m_tuple;
    // =======================================================================
    /// tuple CLID
    CLID m_clid;
    // =======================================================================
    /// tuple 'type'
    Tuples::Type m_type;
    // =======================================================================
    /// reference counter
    size_t m_refCount = 0;
    // =======================================================================
    // helper type to define the actual storage types
    template <typename T>
    class ColumnStorage {
      std::unordered_map<std::string_view, std::pair<T, std::string>> m_map;

    public:
      using element_type = T;
      T* find( std::string_view sv ) {
        auto i = m_map.find( sv );
        return i != m_map.end() ? &i->second.first : nullptr;
      }
      T*                  end() { return nullptr; }
      std::pair<T*, bool> emplace( std::string_view k, T v = T{} ) {
        // tricky way to insert a string_view key which points to the
        // string contained in the mapped type... all just to make find(string_view) work in C++17...
        auto [i, b] = m_map.try_emplace( k, std::move( v ), std::string{ k } );
        if ( !b ) return { static_cast<T*>( nullptr ), false };
        auto nh  = m_map.extract( i );
        nh.key() = nh.mapped().second; // "re-point" key to the string contained value_type
        auto r   = m_map.insert( std::move( nh ) );
        return { &r.position->second.first, r.inserted };
      }
    };

    // =======================================================================
    /// the actual storage of all 'bool' columns
    mutable ColumnStorage<Bool> m_bools;
    // =======================================================================
    /// the actual storage of all 'Int' columns
    mutable ColumnStorage<Char> m_chars;
    // =======================================================================
    /// the actual storage of all 'unsigned int' columns
    mutable ColumnStorage<UChar> m_uchars;
    // =======================================================================
    /// the actual storage of all 'Int' columns
    mutable ColumnStorage<Short> m_shorts;
    // =======================================================================
    /// the actual storage of all 'unsigned int' columns
    mutable ColumnStorage<UShort> m_ushorts;
    // =======================================================================
    /// the actual storage of all 'Int' columns
    mutable ColumnStorage<Int> m_ints;
    // =======================================================================
    /// the actual storage of all 'unsigned int' columns
    mutable ColumnStorage<UInt> m_uints;
    // =======================================================================
    /// the actual storage of all 'long long' columns
    mutable ColumnStorage<LongLong> m_longlongs;
    // =======================================================================
    /// the actual storage of all 'unsigned long long' columns
    mutable ColumnStorage<ULongLong> m_ulonglongs;
    // =======================================================================
    /// the actual storage of all 'Float'   columns
    mutable ColumnStorage<Float> m_floats;
    // =======================================================================
    /// the actual storage of all 'Double'   columns
    mutable ColumnStorage<Double> m_doubles;
    // =======================================================================
    /// the actual storage of all 'Address' columns
    mutable ColumnStorage<Address> m_addresses;
    // =======================================================================
    /// the actual storage of all 'FArray'  columns
    mutable ColumnStorage<FArray> m_farrays;
    // =======================================================================
    /// the actual storage of all 'FArray'  columns (fixed)
    mutable ColumnStorage<FArray> m_arraysf;
    // =======================================================================
    /// the actual storage of all 'FArray'  columns
    mutable ColumnStorage<FMatrix> m_fmatrices;
    // =======================================================================
    /// the actual storage of all 'FMatrix' columns (fixed)
    mutable ColumnStorage<FMatrix> m_matricesf;
    // =======================================================================
    /// all booked types:
    ItemMap m_items;
    // =======================================================================
  };
  // ==========================================================================
} // end of namespace Tuples

// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/TuplePut.h"
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_TUPLEOBJ_H
