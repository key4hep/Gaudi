// $Id: TupleObj.h,v 1.23 2008/10/27 19:22:20 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.23 $
// ============================================================================
#ifndef GAUDIALG_TUPLEOBJ_H
#define GAUDIALG_TUPLEOBJ_H 1
// ============================================================================
// Include files
// ============================================================================
// STD&STL
// ============================================================================
#include <set>
#include <string>
#include <limits>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/VectorMap.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/Tuples.h"
#include "GaudiAlg/Maps.h"
// ============================================================================
// ROOT
// ============================================================================
#include "Math/Point3D.h"
#include "Math/Vector3D.h"
#include "Math/Vector4D.h"
#include "Math/SVector.h"
#include "Math/SMatrix.h"
// ============================================================================
// forward declaration
// ============================================================================
// GaudiKernel
// ============================================================================
class IOpaqueAddress   ;
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
namespace Tuples
{
  // ==========================================================================
  /** @enum Type
   *  the list of available types for ntuples
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *  @date   2004-01-23
   */
  enum Type
    {
      NTUPLE  , // Analysis nTuple
      EVTCOL    // Event Collection
    };
  // ==========================================================================
  /** @enum ErrorCodes
   *
   *  Tuple error codes
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *  @date   2004-01-23
   */
  enum ErrorCodes
    {
      InvalidTuple      = 100 ,
      InvalidColumn           ,
      InvalidOperation        ,
      InvalidObject           ,
      InvalidItem             ,
      TruncateValue     = 200
    };
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
  class GAUDI_API TupleObj
  {
  public:
    // ========================================================================
    /// basic type for int items
    typedef NTuple::Item<bool>                  Bool      ;
    // ========================================================================
    /// basic type for int items
    typedef NTuple::Item<char>                  Char      ;
    // ========================================================================
    /// basic type for unsigned int items
    typedef NTuple::Item<unsigned char>         UChar     ;
    // ========================================================================
    /// basic type for int items
    typedef NTuple::Item<short>                 Short     ;
    // ========================================================================
    /// basic type for unsigned int items
    typedef NTuple::Item<unsigned short>        UShort    ;
    // ========================================================================
    /// basic type for int items
    typedef NTuple::Item<int>                   Int       ;
    // ========================================================================
    /// basic type for unsigned int items
    typedef NTuple::Item<unsigned int>          UInt      ;
    // ========================================================================
    /// basic type for unsigned long long items
    typedef NTuple::Item<long long>             LongLong  ;
    // ========================================================================
    /// basic type for unsigned long long items
    typedef NTuple::Item<unsigned long long>    ULongLong ;
    // ========================================================================
    /// basic type for float items
    typedef NTuple::Item<float>                 Float     ;
    // ========================================================================
    /// basic type for double items
    typedef NTuple::Item<double>                Double    ;
    // ========================================================================
    /// basic type for address items
    typedef NTuple::Item<IOpaqueAddress*>       Address   ;
    // ========================================================================
    /// basic type for array of floats
    typedef NTuple::Array<float>                FArray    ;
    // ========================================================================
    /// basic type for matrix of floats
    typedef NTuple::Matrix<float>               FMatrix   ;
    // ========================================================================
    // the actual type for variable size matrix indices
    typedef unsigned short                      MIndex    ;
    // ========================================================================
    // the map of all items
    typedef std::map<std::string,std::string>   ItemMap   ;
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
    TupleObj
    ( const std::string&    name                          ,
      NTuple::Tuple*        tuple                         ,
      const CLID&           clid = CLID_ColumnWiseTuple   ,
      const Tuples::Type    type = Tuples::NTUPLE         ) ;
    // ========================================================================
  protected:
    // ========================================================================
    /// destructor is protected
    virtual ~TupleObj();
    // ========================================================================
  public:
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
    StatusCode column ( const std::string& name   ,
                        const float        value  );
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
    StatusCode column ( const std::string& name   ,
                        const double       value  );
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
    StatusCode column ( const std::string&   name  ,
                        const short          value );
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
    StatusCode column ( const std::string&  name  ,
                        const short         value ,
                        const short         minv  ,
                        const short         maxv  );
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
    StatusCode column ( const std::string&   name  ,
                        const unsigned short value );
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
    StatusCode column ( const std::string&   name  ,
                        const unsigned short value ,
                        const unsigned short minv  ,
                        const unsigned short maxv  );
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
    StatusCode column ( const std::string&   name  ,
                        const char           value );
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
    StatusCode column ( const std::string&  name  ,
                        const char          value ,
                        const char          minv  ,
                        const char          maxv  );
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
    StatusCode column ( const std::string&   name  ,
                        const unsigned char  value );
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
    StatusCode column ( const std::string&  name  ,
                        const unsigned char value ,
                        const unsigned char minv  ,
                        const unsigned char maxv  );
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
    StatusCode column ( const std::string&   name  ,
                        const int            value );
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
    StatusCode column ( const std::string&   name  ,
                        const int            value ,
                        const int            minv  ,
                        const int            maxv  );
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
    StatusCode column ( const std::string&   name  ,
                        const unsigned int   value );
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
    StatusCode column ( const std::string&   name  ,
                        const unsigned int   value ,
                        const unsigned int   minv  ,
                        const unsigned int   maxv  );
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
    StatusCode column ( const std::string&   name  ,
                        const long           value );
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
    StatusCode column ( const std::string&   name  ,
                        const long           value ,
                        const long           minv  ,
                        const long           maxv  );
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
    StatusCode column ( const std::string&   name  ,
                        const unsigned long  value );
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
    StatusCode column ( const std::string&   name  ,
                        const unsigned long  value ,
                        const unsigned long  minv  ,
                        const unsigned long  maxv  );
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
    StatusCode column ( const std::string&   name  ,
                        const long long      value );
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
    StatusCode column ( const std::string&   name  ,
                        const long long      value ,
                        const long long      minv  ,
                        const long long      maxv  );
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
    StatusCode column ( const std::string&       name  ,
                        const unsigned long long value );
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
    StatusCode column ( const std::string&       name  ,
                        const unsigned long long value ,
                        const unsigned long long minv  ,
                        const unsigned long long maxv  );
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
    StatusCode column ( const std::string& name  ,
                        const signed char  value )
    {
      return column
        ( name  ,
          value ,
          std::numeric_limits<signed char>::min() ,
          std::numeric_limits<signed char>::max() ) ;
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
    StatusCode column ( const std::string& name  ,
                        const bool         value );
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
    StatusCode column ( const std::string&    name    ,
                        IOpaqueAddress*       address ) ;
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
    StatusCode column ( IOpaqueAddress*       address ) ;
    // ========================================================================
  public:
    // ========================================================================
    /** Set the values for several columns simultaneously.
     *  Number of columns is arbitrary, but it should not
     *  be less than number of blank or comma separated tags
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
     *  @warning *ALL* columns are assumed to be of type <tt>double</tt>
     *
     *  @param format blank-separated list of variables,
     *          followed by variable number of arguments.
     *  @attention  All variables are assumed to be <c>double</c> numbers
     *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
     *  @date 2002-10-30
     */
    StatusCode fill( const char* format ... ) ;
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
    template <class DATA>
    StatusCode farray ( const std::string& name          ,
                        DATA               first         ,
                        DATA               last          ,
                        const std::string& length        ,
                        const size_t       maxv          )
    {
      if ( invalid () ) { return InvalidTuple     ; }
      if ( rowWise () ) { return InvalidOperation ; }

      // adjust the length
      if( first + maxv < last )
      {
        Warning( "farray('"+name+"'): array is overflow, skip extra items") ;
        last = first + maxv ;
      }

      // get the length item
      Int* len  = ints( length , 0 , maxv ) ;
      if( 0 == len  ) { return InvalidColumn; }

      // adjust the length item
      *len = last - first ;

      // get the array itself
      FArray* var  = fArray ( name , len ) ;
      if( 0 == var ) { return InvalidColumn ; }

      /// fill the array
      size_t index = 0 ;
      for( ; first != last ; ++first )
      { (*var)[ index ] = (float)(*first) ; ++index ; }

      return StatusCode::SUCCESS ;
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
    StatusCode farray ( const std::string& name          ,
                        const DATA&        data          ,
                        const std::string& length        ,
                        const size_t       maxv          )
    { return farray ( name , data.begin() , data.end() , length , maxv  ) ; }
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
    template <class FUNCTION, class DATA>
    StatusCode farray ( const std::string& name          ,
                        const FUNCTION&    function      ,
                        DATA               first         ,
                        DATA               last          ,
                        const std::string& length        ,
                        const size_t       maxv          )
    {
      if ( invalid () ) { return InvalidTuple     ; }
      if ( rowWise () ) { return InvalidOperation ; }

      // adjust the length
      if( first + maxv < last )
      {
        Warning("farray('"
                + name  + "'): array is overflow, skip extra entries") ;
        last = first + maxv ;
      }

      // get the length item
      Int* len  = ints( length , 0 , maxv ) ;
      if( 0 == len  ) { return InvalidColumn ; }

      // adjust the length
      *len = last - first ;

      // get the array itself
      FArray*  var  = fArray ( name , len ) ;
      if( 0 == var ) { return InvalidColumn ; }

      // fill the array
      size_t index = 0 ;
      for( ; first != last ; ++first )
      { (*var)[ index ] = function( *first )  ; ++index ; }

      return StatusCode::SUCCESS ;
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
     *  tuple->farray( "sqr"          ,   // the first data item name
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
    template <class FUNC1, class FUNC2, class DATA>
    StatusCode farray ( const std::string& name1         ,
                        const FUNC1&       func1         ,
                        const std::string& name2         ,
                        const FUNC2&       func2         ,
                        DATA               first         ,
                        DATA               last          ,
                        const std::string& length        ,
                        const size_t       maxv          )
    {
      if ( invalid () ) { return InvalidTuple     ; }
      if ( rowWise () ) { return InvalidOperation ; }

      // adjust the lenfth
      if( first + maxv < last )
      {
        Warning("farray('"
                + name1 + ","
                + name2 + "'): array is overflow, skip extra entries").ignore() ;
        Warning("farray('"+name1+"'): array is overflow, skip extra items").ignore() ;
        last = first + maxv ;
      }

      // get the length item
      Int* len  = ints ( length , 0 , maxv ) ;
      if ( 0 == len  ) { return InvalidColumn ; }

      // adjust the length
      *len = last - first ;

      // get the array itself
      FArray*  var1  = fArray ( name1 , len ) ;
      if ( 0 == var1 ) { return InvalidColumn ; }

      // get the array itself
      FArray*  var2 = fArray ( name2 , len ) ;
      if ( 0 == var2 ) { return InvalidColumn ; }

      // fill the array
      size_t index = 0 ;
      for( ; first != last ; ++first )
      {
        ( *var1 ) [ index ] = func1 ( *first )  ;
        ( *var2 ) [ index ] = func2 ( *first )  ;
        ++index ;
      }

      return StatusCode::SUCCESS ;
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
     *  tuple->farray( "sqr"          ,   // the first data item name
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
    template <class FUNC1, class FUNC2, class FUNC3, class DATA>
    StatusCode farray ( const std::string& name1         ,
                        const FUNC1&       func1         ,
                        const std::string& name2         ,
                        const FUNC2&       func2         ,
                        const std::string& name3         ,
                        const FUNC3&       func3         ,
                        DATA               first         ,
                        DATA               last          ,
                        const std::string& length        ,
                        const size_t       maxv          )
    {
      if ( invalid () ) { return InvalidTuple     ; }
      if ( rowWise () ) { return InvalidOperation ; }

      // adjust the length
      if( first + maxv < last )
      {
        Warning("farray('"
                + name1 + ","
                + name2 + ","
                + name3 + "'): array is overflow, skip extra entries").ignore() ;
        last = first + maxv ;
      }

      // get the length item
      Int* len  = ints ( length , 0 , maxv ) ;
      if( 0 == len   ) { return InvalidColumn ; }

      // adjust the length
      *len = last - first ;

      // get the array itself
      FArray*  var1  = fArray ( name1 , len ) ;
      if( 0 == var1 ) { return InvalidColumn  ; }

      // get the array itself
      FArray*  var2 = fArray ( name2 , len ) ;
      if( 0 == var2 ) { return InvalidColumn ; }

      // get the array itself
      FArray*  var3 = fArray ( name3 , len ) ;
      if( 0 == var3 ) { return InvalidColumn ; }

      // fill the array
      size_t index = 0 ;
      for( ; first != last ; ++first )
      {
        ( *var1 ) [ index ] = (float)func1 ( *first )  ;
        ( *var2 ) [ index ] = (float)func2 ( *first )  ;
        ( *var3 ) [ index ] = (float)func3 ( *first )  ;
        ++index ;
      }
      return StatusCode::SUCCESS ;
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
     *  tuple->farray( "sqr"          ,   // the first data item name
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
    template <class FUNC1, class FUNC2, class FUNC3, class FUNC4, class DATA>
    StatusCode farray ( const std::string& name1         ,
                        const FUNC1&       func1         ,
                        const std::string& name2         ,
                        const FUNC2&       func2         ,
                        const std::string& name3         ,
                        const FUNC3&       func3         ,
                        const std::string& name4         ,
                        const FUNC4&       func4         ,
                        DATA               first         ,
                        DATA               last          ,
                        const std::string& length        ,
                        const size_t       maxv          )
    {
      if ( invalid () ) { return InvalidTuple     ; }
      if ( rowWise () ) { return InvalidOperation ; }

      // adjust the length
      if( first + maxv < last )
      {
        Warning("farray('"
                + name1 + ","
                + name2 + ","
                + name3 + ","
                + name4 + "'): array is overflow, skip extra entries").ignore() ;
        last = first + maxv ;
      }

      // get the length item
      Int* len  = ints ( length , 0 , maxv ) ;
      if( 0 == len  ) { return InvalidColumn ; }

      // adjust the length
      *len = last - first ;

      // get the array itself
      FArray*  var1  = fArray ( name1 , len ) ;
      if( 0 == var1 ) { return InvalidColumn ; }

      // get the array itself
      FArray*  var2 = fArray ( name2 , len ) ;
      if( 0 == var2 ) { return InvalidColumn ; }

      // get the array itself
      FArray*  var3 = fArray ( name3 , len ) ;
      if( 0 == var3 ) { return InvalidColumn ; }

      // get the array itself
      FArray*  var4 = fArray ( name4 , len ) ;
      if( 0 == var4 ) { return InvalidColumn ; }

      // fill the array
      size_t index = 0 ;
      for( ; first != last ; ++first )
      {
        ( *var1 ) [ index ] = static_cast<float> ( func1 ( *first ) );
        ( *var2 ) [ index ] = static_cast<float> ( func2 ( *first ) );
        ( *var3 ) [ index ] = static_cast<float> ( func3 ( *first ) );
        ( *var4 ) [ index ] = static_cast<float> ( func4 ( *first ) );
        ++index ;
      }

      return StatusCode::SUCCESS ;
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
    StatusCode fmatrix ( const std::string& name    ,
                         const MATRIX&      data    ,
                         size_t             rows    ,
                         const MIndex&      cols    ,
                         const std::string& length  ,
                         const size_t       maxv    )
    {
      if ( invalid () ) { return InvalidTuple     ; }
      if ( rowWise () ) { return InvalidOperation ; }

      // adjust the length
      if ( rows >= maxv )
      {
        Warning ( "fmatrix('"+name+"'): matrix is overflow, skip extra items").ignore() ;
        rows = ( 0 < maxv ) ? ( maxv - 1 ) : 0 ;
      }

      // get the length item
      Int* len  = ints( length , 0 , maxv ) ;
      if ( 0 == len  ) { return InvalidColumn; }

      // adjust the length item
      *len = rows ;

      // get the array itself
      FMatrix* var  = fMatrix ( name , len  , cols ) ;
      if ( 0 == var ) { return InvalidColumn ; }

      /// fill the matrix
      for ( size_t iCol = 0 ; iCol < cols ; ++iCol )
      {
        for ( MIndex iRow = 0 ; iRow < rows ; ++iRow )
        { (*var)[ iRow ] [ iCol ] = (float)(data[ iRow ][ iCol ]) ; }
      }

      return StatusCode::SUCCESS ;
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
    StatusCode fmatrix ( const std::string& name   ,
                         DATA               first  ,
                         DATA               last   ,
                         const MIndex&      cols   ,
                         const std::string& length ,
                         const size_t       maxv   )
    {
      if ( invalid () ) { return InvalidTuple     ; }
      if ( rowWise () ) { return InvalidOperation ; }

      // adjust the length
      if ( first + maxv < last )
      {
        Warning("fmatrix('"+name+"'): matrix is overflow, skip extra items").ignore() ;
        last = first + maxv ;
      }

      // get the length item
      Int* len  = ints( length , 0 , maxv ) ;
      if ( 0 == len  ) { return InvalidColumn; }

      // adjust the length item
      *len = last - first ;

      // get the array itself
      FMatrix* var  = fMatrix ( name , len , cols ) ;
      if ( 0 == var ) { return InvalidColumn ; }

      /// fill the matrix
      size_t iRow = 0 ;
      for ( ; first != last ; ++first )
      {
        //
        for ( MIndex iCol = 0 ; iCol < cols ; ++iCol )
        { (*var)[ iRow ] [ iCol ] = (float)((*first)[ iCol ]) ; }
        //
        ++iRow ;
      }

      return StatusCode::SUCCESS ;
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
    template <class FUN,class DATA>
    StatusCode fmatrix ( const std::string& name   ,
                         FUN                funF   ,
                         FUN                funL   ,
                         DATA               first  ,
                         DATA               last   ,
                         const std::string& length ,
                         const size_t       maxv   )
    {
      if ( invalid () ) { return InvalidTuple     ; }
      if ( rowWise () ) { return InvalidOperation ; }

      // adjust the length
      if ( first + maxv < last )
      {
        Warning("fmatrix('"+name+"'): matrix is overflow, skip extra items").ignore() ;
        last = first + maxv ;
      }

      // get the length item
      Int* len  = ints( length , 0 , maxv ) ;
      if ( 0 == len  ) { return InvalidColumn; }

      // adjust the length item
      *len = last - first ;

      // get the array itself
      const size_t cols = funL - funF ;
      FMatrix* var  = fMatrix ( name , len , cols ) ;
      if ( 0 == var ) { return InvalidColumn ; }

      /// fill the matrix
      size_t iRow = 0 ;
      for ( ; first != last ; ++first )
      {
        //
        for ( FUN fun = funF ; fun < funL ; ++fun )
        { (*var)[ iRow ] [ fun - funF ] = (float)((*fun) ( *first )) ; }
        //
        ++iRow;
      }

      return StatusCode::SUCCESS ;
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
    StatusCode array ( const std::string& name          ,
                       DATA               first         ,
                       DATA               last          )

    {
      if ( invalid () ) { return InvalidTuple     ; }
      if ( rowWise () ) { return InvalidOperation ; }

      // get the length (fixed!)
      const size_t length = last - first ;

      // get the array itself
      FArray* var  = fArray ( name , length ) ;
      if ( 0 == var ) { return InvalidColumn ; }

      /// fill the array
      size_t iCol = 0 ;
      for ( ; first != last ; ++first )
      { (*var)[ iCol ] = (float)(*first) ; ++iCol ; }

      return StatusCode::SUCCESS ;
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
    StatusCode array ( const std::string& name   ,
                       const ARRAY&       data   ,
                       const MIndex&      length )
    {
      if ( invalid () ) { return InvalidTuple     ; }
      if ( rowWise () ) { return InvalidOperation ; }

      // get the array itself
      FArray* var  = fArray ( name , length ) ;
      if ( 0 == var ) { return InvalidColumn ; }

      /// fill the array
      for ( size_t index = 0 ; index < length ; ++index )
      { (*var)[ index ] = (float) data[index] ; }

      return StatusCode::SUCCESS ;
    }
    // =======================================================================
    /** fill N-Tuple with fixed-size array
     *
     *  "ARRAY" is any sequence, which supports
     *   ARRAY::begin() and ARRAY::end() protocol, e.g.
     *
     *   - std::vector<TYPE>
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
    StatusCode array ( const std::string& name  ,
                       const ARRAY&       data  )
    { return array ( name , data.begin() , data.end() ) ; }
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
    StatusCode matrix ( const std::string& name  ,
                        const MATRIX&      data  ,
                        const MIndex&      rows  ,
                        const MIndex&      cols  )
    {
      if ( invalid () ) { return InvalidTuple     ; }
      if ( rowWise () ) { return InvalidOperation ; }

      // get the matrix itself
      FMatrix* var  = fMatrix ( name , rows , cols ) ;
      if ( 0 == var ) { return InvalidColumn ; }

      /// fill the matrix
      for ( size_t iCol = 0 ; iCol < cols ; ++iCol )
      {
        for ( size_t iRow = 0 ; iRow < rows ; ++iRow )
        { (*var)[iRow][iCol] = (float)(data[iRow][iCol]) ; }
      };
      return StatusCode::SUCCESS ;
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
    StatusCode column
    ( const std::string& name ,
      const ROOT::Math::LorentzVector<TYPE>& v )
    {
      if ( invalid() ) { return InvalidTuple ; }
      // fill all separate columns:
      StatusCode sc1 = this -> column ( name + "E" , v.E  () ) ;
      StatusCode sc2 = this -> column ( name + "X" , v.Px () ) ;
      StatusCode sc3 = this -> column ( name + "Y" , v.Py () ) ;
      StatusCode sc4 = this -> column ( name + "Z" , v.Pz () ) ;
      return
        sc1.isFailure () ? sc1 :
        sc2.isFailure () ? sc2 :
        sc3.isFailure () ? sc3 :
        sc4.isFailure () ? sc4 : StatusCode(StatusCode::SUCCESS) ;
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
    template <class TYPE,class TAG>
    StatusCode column
    ( const std::string& name ,
      const ROOT::Math::DisplacementVector3D<TYPE,TAG>& v )
    {
      if ( invalid() ) { return InvalidTuple ; }
      /// fill separate columns
      StatusCode sc1 = this -> column ( name + "X" , v.X () ) ;
      StatusCode sc2 = this -> column ( name + "Y" , v.Y () ) ;
      StatusCode sc3 = this -> column ( name + "Z" , v.Z () ) ;
      return
        sc1.isFailure () ? sc1 :
        sc2.isFailure () ? sc2 :
        sc3.isFailure () ? sc3 : StatusCode(StatusCode::SUCCESS) ;
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
    template <class TYPE,class TAG>
    StatusCode column
    ( const std::string& name ,
      const ROOT::Math::PositionVector3D<TYPE,TAG>& v )
    {
      if ( invalid() ) { return InvalidTuple ; }
      /// fill separate columns
      StatusCode sc1 = this -> column ( name + "X" , v.X () ) ;
      StatusCode sc2 = this -> column ( name + "Y" , v.Y () ) ;
      StatusCode sc3 = this -> column ( name + "Z" , v.Z () ) ;
      return
        sc1.isFailure () ? sc1 :
        sc2.isFailure () ? sc2 :
        sc3.isFailure () ? sc3 : StatusCode(StatusCode::SUCCESS) ;
    }
    // =======================================================================
    /** shortcut to put SVector into N-tuple:
     *
     *  @code
     *
     *  ROOT::Math::SVector<double,15> vct = ... ;
     *
     *  Tuple tuple = nTuple("My N-Tuple") ;
     *
     *  // put the vector into N-Tuple:
     *  tuple -> array ( "v" , vct ) ;
     *
     *  @endcode
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-11-26
     */
    template <class TYPE,unsigned int DIM>
    StatusCode array
    ( const std::string&                   name ,
      const ROOT::Math::SVector<TYPE,DIM>& vect )
    {
      return this->array( name , vect.begin() , vect.end() ) ;
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
    template <class TYPE,unsigned int D1,unsigned int D2,class REP>
    StatusCode matrix
    ( const std::string&                         name ,
      const ROOT::Math::SMatrix<TYPE,D1,D2,REP>& mtrx )
    {
      if ( invalid () ) { return InvalidTuple     ; }
      if ( rowWise () ) { return InvalidOperation ; }

      // get the matrix itself
      FMatrix* var  = fMatrix ( name , (MIndex)D1 , (MIndex)D2 ) ;
      if ( 0 == var   ) { return InvalidColumn ; }

      /// fill the matrix
      for ( size_t iCol = 0 ; iCol < D2 ; ++iCol )
      {
        for ( size_t iRow = 0 ; iRow < D1 ; ++iRow )
        { (*var)[iRow][iCol] = (float) mtrx(iRow,iCol) ; }
      };

      return StatusCode::SUCCESS ;
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
    StatusCode fmatrix
    ( const std::string&                      name       ,
      const GaudiUtils::VectorMap<KEY,VALUE>& info       ,
      const std::string&                      length     ,
      const size_t                            maxv = 100 )
    {

      if ( invalid () ) { return InvalidTuple     ; }
      if ( rowWise () ) { return InvalidOperation ; }

      typename GaudiUtils::VectorMap<KEY,VALUE>::const_iterator begin = info.begin () ;
      typename GaudiUtils::VectorMap<KEY,VALUE>::const_iterator end   = info.end   () ;

      // adjust the length
      if ( maxv < info.size() )
      {
        Warning("fmatrix('"+name+"'): matrix is overflow, skip extra items") ;
        end = begin + maxv ;
      } ;

      // get the length item
      Int* len  = ints( length , 0 , maxv ) ;
      if ( 0 == len  ) { return InvalidColumn; }

      // adjust the length item
      *len = end - begin ;

      // get the array itself
      FMatrix* var  = fMatrix ( name , len , 2 ) ;
      if ( 0 == var ) { return InvalidColumn ; }

      /// fill the matrix
      size_t iRow = 0 ;
      for ( ; begin != end ; ++begin)
      {
        //
        (*var)[iRow][0] = (float) begin->first   ;
        (*var)[iRow][1] = (float) begin->second  ;
        //
        ++iRow ;
      } ;

      return StatusCode::SUCCESS ;
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
    StatusCode put
    ( const std::string& name , const TYPE*  obj ) ;
    // =======================================================================
  public:
    // =======================================================================
    /** write a record to NTuple
     *  @return status code
     */
    StatusCode write () ;
    // =======================================================================
    /// get the name
    const std::string&  name() const { return m_name ; }
    // =======================================================================
    /** provide the access to underlying Gaudi N-tuple
     *  @return pointer to Gaudi N-tuple object
     */
    NTuple::Tuple* tuple() const  { return m_tuple ; }
    // =======================================================================
    /** return the reference counter
     *  @return current reference counter
     */
    unsigned long refCount() const { return   m_refCount ; }
    // =======================================================================
    /** add the reference to TupleObj
     *  @return current reference counter
     */
    unsigned long addRef  ()       { return ++m_refCount ; }
    // =======================================================================
    /** release the reference to TupleObj
     *  if reference counter becomes zero,
     *  object will be automatically deleted
     */
    void          release () ;
    // =======================================================================
    /// accessor to the N-Tuple CLID
    const CLID&        clid() const { return m_clid ; }
    // =======================================================================
    /// accessor to the N-Tuple type
    Tuples::Type       type() const { return m_type ; }
    // =======================================================================
    /// column wise NTuple ?
    bool columnWise() const { return CLID_ColumnWiseTuple == clid() ; }
    // =======================================================================
    /// row wise NTuple ?
    bool rowWise   () const { return CLID_RowWiseTuple    == clid() ; }
    // =======================================================================
    /// Event collection ?
    bool evtColType() const { return Tuples::EVTCOL       == type()  ; }
    // =======================================================================
    /// valid pointer to tuple ?
    bool valid     () const { return 0 != tuple() ; }
    // =======================================================================
    /// invalid pointer to tuple ?
    bool invalid   () const { return ! valid()    ; }
    // =======================================================================
  public:
    // =======================================================================
    /** add the item name into the list of known items
     *  @param name the name of the item
     *  @param type the type of the item
     *  @return true if the name is indeed added
     */
    bool addItem ( const std::string& name ,
                   const std::string& type )
    { return m_items.insert ( std::make_pair ( name , type ) ).second ; }
    // =======================================================================
    /** check the uniqueness of the name
     *  @param name the name of the item
     *  @return true if the name is indeed unique
     */
    bool goodItem  ( const std::string& name ) const
    { return m_items.end() == m_items.find ( name ) ; }
    // =======================================================================
    /// get the full list of booked items
    const ItemMap& items() const { return m_items ; }
    // =======================================================================
  public:
    // =======================================================================
    virtual StatusCode Error
    ( const std::string& msg ,
      const StatusCode   sc  = StatusCode::FAILURE ) const = 0 ;
    // =======================================================================
    virtual StatusCode Warning
    ( const std::string& msg ,
      const StatusCode   sc  = StatusCode::FAILURE ) const = 0 ;
    // =======================================================================
  private:
    // =======================================================================
    /// get the column
    Bool*    bools      ( const std::string& name ) ;
    // =======================================================================
    /// get the column
    Float*   floats     ( const std::string& name ) ;
    // =======================================================================
    /// get the column
    Double*  doubles    ( const std::string& name ) ;
    // =======================================================================
    /// get the column
    Char*   chars     ( const std::string& name ) ;
    // =======================================================================
    /// get the column
    Char*   chars     ( const std::string& name ,
                        const char         minv ,
                        const char         maxv ) ;
    // =======================================================================
    /// get the column
    UChar*  uchars    ( const std::string& name ) ;
    // =======================================================================
    /// get the column
    UChar*  uchars    ( const std::string&  name ,
                        const unsigned char minv ,
                        const unsigned char maxv ) ;
    // =======================================================================
    /// get the column
    Short*   shorts     ( const std::string& name ) ;
    // =======================================================================
    /// get the column
    Short*   shorts     ( const std::string& name ,
                          const short        minv ,
                          const short        maxv ) ;
    // =======================================================================
    /// get the column
    UShort*   ushorts   ( const std::string& name ) ;
    // =======================================================================
    /// get the column
    UShort*   ushorts   ( const std::string& name ,
                          const unsigned short minv ,
                          const unsigned short maxv ) ;
    // =======================================================================
    /// get the column
    Int*     ints       ( const std::string& name ) ;
    // =======================================================================
    /// get the column
    Int*     ints       ( const std::string& name ,
                          const int          minv ,
                          const int          maxv ) ;
    // =======================================================================
    /// get the column
    UInt*     uints     ( const std::string& name ) ;
    // =======================================================================
    /// get the column
    UInt*     uints     ( const std::string& name ,
                          const unsigned int minv ,
                          const unsigned int maxv ) ;
    // =======================================================================
    /// get the column
    LongLong* longlongs ( const std::string& name );
    // =======================================================================
    /// get the column
    LongLong* longlongs ( const std::string& name ,
                          const long long    minv ,
                          const long long    maxv ) ;
    // =======================================================================
    /// get the column
    ULongLong* ulonglongs ( const std::string& name ) ;
    // =======================================================================
    /// get the column
    ULongLong* ulonglongs ( const std::string&       name ,
                            const unsigned long long minv ,
                            const unsigned long long maxv ) ;
    // =======================================================================
    /// get the column
    FArray*  fArray     ( const std::string& name ,
                          Int*               item ) ;
    // =======================================================================
    /// get the column
    FArray*  fArray     ( const std::string& name ,
                          const MIndex&      rows ) ;
    // =======================================================================
    /// get the column
    Address* addresses  ( const std::string& name ) ;
    // =======================================================================
    /// get the column
    FMatrix* fMatrix    ( const std::string& name ,
                          Int*               item ,
                          const MIndex&      cols ) ;
    // =======================================================================
    /// get the column
    FMatrix* fMatrix    ( const std::string& name ,
                          const MIndex&      rows ,
                          const MIndex&      cols ) ;
    // =======================================================================
  private:
    // =======================================================================
    /// the default constructor is disabled
    TupleObj () ;
    // =======================================================================
    /// copy constructor is disabled
    TupleObj            ( const TupleObj& ) ;
    // =======================================================================
    /// assignment is disabled
    TupleObj& operator= ( const TupleObj& ) ;
    // =======================================================================
  private:
    // =======================================================================
    /// the actual storage type for short columns
    typedef GaudiUtils::HashMap<std::string,Bool*>   Bools;
    // =======================================================================
    /// the actual storage type for short columns
    typedef GaudiUtils::HashMap<std::string,Char*>   Chars;
    // =======================================================================
    /// the actual storage type for unsigned short columns
    typedef GaudiUtils::HashMap<std::string,UChar*>  UChars;
    // =======================================================================
    /// the actual storage type for short columns
    typedef GaudiUtils::HashMap<std::string,Short*>  Shorts;
    // =======================================================================
    /// the actual storage type for unsigned short columns
    typedef GaudiUtils::HashMap<std::string,UShort*> UShorts;
    // =======================================================================
    /// the actual storage type for integer columns
    typedef GaudiUtils::HashMap<std::string,Int*>    Ints;
    // =======================================================================
    /// the actual storage type for unsigned integer columns
    typedef GaudiUtils::HashMap<std::string,UInt*>   UInts;
    // =======================================================================
    /// the actual storage type for longlong columns
    typedef GaudiUtils::HashMap<std::string,LongLong*> LongLongs;
    // =======================================================================
    /// the actual storage type for ulonglong columns
    typedef GaudiUtils::HashMap<std::string,ULongLong*> ULongLongs;
    // =======================================================================
    /// the actual storage type for float columns
    typedef GaudiUtils::HashMap<std::string,Float*>   Floats;
    // =======================================================================
    /// the actual storage type for float columns
    typedef GaudiUtils::HashMap<std::string,Double*>  Doubles;
    // =======================================================================
    /// the actual storage type for address columns
    typedef GaudiUtils::HashMap<std::string,Address*> Addresses;
    // =======================================================================
    /// the actual storage type for array columns
    typedef GaudiUtils::HashMap<std::string,FArray*>  FArrays;
    // =======================================================================
    /// the actual storage type for matrix columns
    typedef GaudiUtils::HashMap<std::string,FMatrix*> FMatrices;
    // =======================================================================
  private:
    // =======================================================================
    /// name
    std::string            m_name       ;
    // =======================================================================
    /// tuple itself
    NTuple::Tuple*         m_tuple      ;
    // =======================================================================
    /// tuple CLID
    CLID                   m_clid       ;
    // =======================================================================
    /// tuple 'type'
    Tuples::Type           m_type       ;
    // =======================================================================
    /// reference counter
    size_t                 m_refCount   ;
    // =======================================================================
    /// the actual storage of all 'bool' columns
    mutable Bools          m_bools      ;
    // =======================================================================
    /// the actual storage of all 'Int' columns
    mutable Chars          m_chars      ;
    // =======================================================================
    /// the actual storage of all 'unsigned int' columns
    mutable UChars         m_uchars     ;
    // =======================================================================
    /// the actual storage of all 'Int' columns
    mutable Shorts         m_shorts     ;
    // =======================================================================
    /// the actual storage of all 'unsigned int' columns
    mutable UShorts        m_ushorts    ;
    // =======================================================================
    /// the actual storage of all 'Int' columns
    mutable Ints           m_ints       ;
    // =======================================================================
    /// the actual storage of all 'unsigned int' columns
    mutable UInts          m_uints      ;
    // =======================================================================
    /// the actual storage of all 'longlong' columns
    mutable LongLongs      m_longlongs  ;
    // =======================================================================
    /// the actual storage of all 'ulonglong' columns
    mutable ULongLongs     m_ulonglongs ;
    // =======================================================================
    /// the actual storage of all 'Float'   columns
    mutable Floats         m_floats     ;
    // =======================================================================
    /// the actual storage of all 'Double'   columns
    mutable Doubles        m_doubles    ;
    // =======================================================================
    /// the actual storage of all 'Address' columns
    mutable Addresses      m_addresses  ;
    // =======================================================================
    /// the actual storage of all 'FArray'  columns
    mutable FArrays        m_farrays    ;
    // =======================================================================
    /// the actual storage of all 'FArray'  columns (fixed)
    mutable FArrays        m_arraysf    ;
    // =======================================================================
    /// the actual storage of all 'FArray'  columns
    mutable FMatrices      m_fmatrices  ;
    // =======================================================================
    /// the actual storage of all 'FMatrix' columns (fixed)
    mutable FMatrices      m_matricesf  ;
    // =======================================================================
    /// all booked types:
    ItemMap                m_items      ;
    // =======================================================================
  } ;
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
// ============================================================================
