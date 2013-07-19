// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <algorithm>
#include <numeric>
#include <limits>
// ============================================================================
// CLHEP
// ============================================================================
#include "CLHEP/Matrix/Matrix.h"
#include "CLHEP/Matrix/Vector.h"
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/Tuples.h"
#include "GaudiAlg/GaudiTupleAlg.h"
// ============================================================================

// Handle CLHEP 2.0.x move to CLHEP namespace
namespace CLHEP { }
using namespace CLHEP;

// ============================================================================
/** @class TupleAlg
 *
 *  Example of usage GaudiTupleAlg base class
 *
 *  @see GaudiTupleAlg
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
 *  @date 2005-05-01
 */
// ============================================================================
class TupleAlg : public GaudiTupleAlg
{
public:
  /// initialize the algorithm
  StatusCode initialize ()
  {
    StatusCode sc = GaudiTupleAlg::initialize() ;
    if ( sc.isFailure() ) { return sc ; }
    // check for random numbers service
    Assert ( randSvc() != 0, "Random Service is not available!");
    //
    return StatusCode::SUCCESS ;
  }
  /** the only one essential method
   *  @see IAlgoruthm
   */
  StatusCode execute    () ;

  /** standard constructor
   *  @param name algorithm instance name
   *  @param pSvc pointer to Service Locator
   */
  TupleAlg
  ( const std::string& name ,
    ISvcLocator*       pSvc )
    : GaudiTupleAlg ( name , pSvc ) { }
  // destructor
  virtual ~TupleAlg() {} ;
private:
  // default constructor is disabled
  TupleAlg() ;
  // copy constructor is disabled
  TupleAlg( const TupleAlg& ) ;
  // assignment operator is disabled
  TupleAlg& operator=( const TupleAlg& ) ;
  // Make a random generator for a type
  template< class T >
  T randomRange()
  {
    const T min = std::numeric_limits<T>::min();
    const T max = std::numeric_limits<T>::max();
    return min + (T)( (max-min) *
                      (double)(Rndm::Numbers( randSvc(), Rndm::Flat(0,1) )()) );
  }

};


// ============================================================================
/** the only one essential method
 *  @see IAlgoruthm
 */
// ============================================================================
StatusCode TupleAlg::execute()
{
  /// avoid long names
  using namespace Tuples ;

  Rndm::Numbers gauss   ( randSvc() , Rndm::Gauss       (   0.0 ,  1.0 ) ) ;
  Rndm::Numbers flat    ( randSvc() , Rndm::Flat        ( -10.0 , 10.0 ) ) ;
  Rndm::Numbers expo    ( randSvc() , Rndm::Exponential (   1.0        ) ) ;
  Rndm::Numbers breit   ( randSvc() , Rndm::BreitWigner (   0.0 ,  1.0 ) ) ;
  Rndm::Numbers poisson ( randSvc() , Rndm::Poisson     (   2.0        ) ) ;
  Rndm::Numbers binom   ( randSvc() , Rndm::Binomial    (   8   , 0.25 ) ) ;

  // ==========================================================================
  // book and fill simple Row-wise NTuple with scalar items only
  // use a numeric ID
  // ==========================================================================
  Tuple tuple1 = nTuple ( 1 , "Trivial Row-Wise Tuple" , CLID_RowWiseTuple ) ;

  // fill N-Tuple with double/float numbers:
  tuple1 -> column ( "gauss"  ,        gauss   () ) ;
  tuple1 -> column ( "flat"   ,        flat    () ) ;
  tuple1 -> column ( "expo"   ,        expo    () ) ;
  tuple1 -> column ( "breit"  ,        breit   () ) ;

  // fill N-Tuple with integer numbers:
  tuple1 -> column ( "poiss"  , (int)  poisson () ) ;
  tuple1 -> column ( "binom"  , (int)  binom   () ) ;

  // fill N-Tuple with "reduced" integer numbers:
  tuple1 -> column ( "pois2"  , (int)  poisson () , 0 , 14 ) ;
  tuple1 -> column ( "bino2"  , (int)  binom   () , 0 , 14 ) ;

  // fill N-Tuple with "boolean" numbers:
  tuple1 -> column ( "poisb"  ,  poisson ()  > 0.0) ;

  tuple1->write() ;

  // ==========================================================================
  // book and fill Column-wise NTuple with "identical" content
  // use a literal ID instead of a numeric one
  // ==========================================================================
  Tuple tuple2 = nTuple ( "two" , "Column-Wise Tuple" ) ;

  // fill N-Tuple with double/float numbers:
  tuple2 -> column ( "gauss"  ,        gauss   () ) ;
  tuple2 -> column ( "flat"   ,        flat    () ) ;
  tuple2 -> column ( "expo"   ,        expo    () ) ;
  tuple2 -> column ( "breit"  ,        breit   () ) ;

  // fill N-Tuple with integer numbers:
  tuple2 -> column ( "poiss"  , (int)  poisson () ) ;
  tuple2 -> column ( "binom"  , (int)  binom   () ) ;

  // fill N-Tuple with "reduced" integer numbers:
  tuple2 -> column ( "pois2"  , (int)  poisson () , 0 , 10 ) ;
  tuple2 -> column ( "bino2"  , (int)  binom   () , 0 , 10 ) ;

  // fill N-Tuple with "boolean" numbers:
  tuple2 -> column ( "poisb"  , poisson () > 0.0 ) ;

  tuple2 -> write () ;


  // ==========================================================================
  // book and fill Column-wise NTuple with "fixed"-size arrays/vectors
  // use a numeric ID
  // ==========================================================================
  Tuple tuple3 = nTuple ( 3 , "Fixed-size arrays/vectors" ) ;

  { // fill using iterator/sequence protocol
    const size_t nCol =  50 ;
    float array[nCol] ;
    for ( size_t i = 0 ; i < nCol ; ++i ) { array[i] = (float)flat () ; }

    // fill with simple array/vector (fixed size):
    tuple3 -> array ( "arflat"     ,
                      array        ,
                      array + nCol ) ;
  }

  {
    typedef std::vector<double> Array ;
    const size_t nCol =  62 ;
    Array array(nCol) ;
    for ( size_t i = 0 ; i < array.size() ; ++i )
    { array[i] = expo() ; }

    // fill with simple array/vector (fixed size):
    tuple3 -> array ( "arexpo" , array ) ;
  }

  { // fill with the explicit usage of sequence length
    const size_t nCol =  42 ;
    double array[nCol] ;
    for ( size_t i = 0 ; i < nCol ; ++i ) { array[i] = gauss() ; }

    // fill with simple array/vector (fixed size):
    tuple3 -> array ( "argau" ,
                      array   ,
                      nCol    ) ;
  }

  { // fill with the explicit usage of sequence length
    const size_t nCol =  42 ;
    CLHEP::HepVector array(nCol);
    for ( size_t i = 0 ; i < nCol ; ++i ) { array[i] = gauss() ; }

    // fill with simple array/vector (fixed size):
    tuple3 -> array ( "argau2" ,
                      array    ,
                      nCol     ) ;
  }

  tuple3 -> write() ;

  // ==========================================================================
  // book and fill Column-wise NTuple with "fixed"-size matrices
  // use a literal ID in a sub-dir instead of a numeric one
  // ==========================================================================
  Tuple tuple4 = nTuple ( "subdir/four" , "Fixed-size matrices" ) ;

  {
    // fill with simple 2D-array
    const size_t nRow = 15 ;
    const size_t nCol =  5 ;

    double mtrx[nRow][nCol] ;

    for ( size_t iRow = 0 ; iRow < nRow ; ++iRow )
    { for ( size_t iCol = 0 ; iCol < nCol ; ++iCol )
    { mtrx[iRow][iCol] = gauss() ; } }

    tuple4 -> matrix ( "mgau" ,
                       mtrx ,
                       nRow ,
                       nCol ) ;

  };

  {
    // fill with simple "pseudo-matrix"
    typedef std::vector<double> Row  ;
    typedef std::vector<Row>    Mtrx ;

    const size_t nRow = 26 ;
    const size_t nCol =  4 ;

    Mtrx mtrx ( nRow , Row( nCol ) ) ;

    for ( size_t iRow = 0 ; iRow < nRow ; ++iRow )
    { for ( size_t iCol = 0 ; iCol < nCol ; ++iCol )
    { mtrx[iRow][iCol] = flat() ; } }

    tuple4 -> matrix ( "mflat"   ,
                       mtrx   ,
                       nRow   ,
                       nCol   ) ;

  };

  {
    // fill with simple CLHEP matrix
    const size_t nRow =  13 ;
    const size_t nCol =   3 ;
    CLHEP::HepMatrix mtrx( nRow , nCol ) ;
    for ( int iCol = 0 ; iCol < mtrx.num_col() ; ++iCol )
    { for ( int iRow = 0 ; iRow < mtrx.num_row() ; ++iRow )
    { mtrx[iRow][iCol] = expo() ; } }

    tuple4 -> matrix ( "mexpo"        ,
                       mtrx           ,
                       mtrx.num_row() ,
                       mtrx.num_col() ) ;

  };

  tuple4 -> write() ;

  // ==========================================================================
  // book and fill Column-wise NTuple with variable-size arrays/vectors
  // ==========================================================================
  Tuple tuple5 = nTuple ( 5 , "Variable-size arrays/vectors" ) ;

  { // fill with the constent of vector
    const size_t num = (size_t) poisson() ;

    typedef std::vector<double> Array ;
    Array array ;
    std::generate_n ( std::back_inserter( array ) , num , gauss ) ;

    tuple5 -> farray ( "arr"         ,
                       array.begin() ,
                       array.end  () ,
                       "Len1"        ,
                       100           ) ;
  }

#ifndef WIN32
  { // fill with the function of vector
    const size_t num = (size_t) poisson() ;

    typedef std::vector<double> Array ;
    Array array ;
    std::generate_n ( std::back_inserter( array ) , num , gauss ) ;

    tuple5 -> farray ( "sinar"        ,
                       useFun ( sin ) ,
                       "cosar"        ,
                       useFun ( cos ) ,
                       "tanar"        ,
                       useFun ( (double (*)(double))tan ) ,
                       //
                       array.begin () ,
                       array.end   () ,
                       "Len2"         ,
                       100            ) ;
  }

#else
  { // fill with the function of vector
    const size_t num = (size_t) poisson() ;

    typedef std::vector<double> Array ;
    Array array ;
    std::generate_n ( std::back_inserter( array ) , num , gauss ) ;

    tuple5 -> farray ( "sinar"        ,
                       useFun ( (double (*)(double))sin ) ,
                       "cosar"        ,
                       useFun ( (double (*)(double))cos ) ,
                       "tanar"        ,
                       useFun ( (double (*)(double))tan ) ,
                       //
                       array.begin () ,
                       array.end   () ,
                       "Len2"         ,
                       100            ) ;
  }
#endif

  tuple5 -> write() ;

  // ==========================================================================
  // book and fill Column-wise NTuple with variable-size matrices
  // ==========================================================================
  Tuple tuple6 = nTuple ( "six" , "Variable-size matrices" ) ;

  { // fill with the matrix
    const size_t num = (size_t) poisson() ;
    typedef std::vector<double> Row  ;
    typedef std::vector<Row>    Mtrx ;
    const size_t nCol = 15 ;

    Mtrx mtrx ( num , Row( nCol ) ) ;

    for ( size_t iRow = 0 ; iRow < num ; ++iRow )
    { for ( size_t iCol = 0 ; iCol < nCol ; ++iCol )
    { mtrx [iRow][iCol] = gauss() ; } }

    tuple6 -> fmatrix ( "mgau"       ,
                        mtrx.begin() ,
                        mtrx.end  () ,
                        nCol         ,
                        "Len1"       ,
                        100          ) ;

  };

  { // fill with the matrix
    const size_t num = (size_t) poisson() ;
    typedef std::vector<double> Row  ;
    typedef std::vector<Row>    Mtrx ;
    const size_t nCol = 15 ;

    Mtrx mtrx ( num , Row( nCol ) ) ;

    for ( size_t iRow = 0 ; iRow < num ; ++iRow )
    { for ( size_t iCol = 0 ; iCol < nCol ; ++iCol )
    { mtrx [iRow][iCol] = expo() ; } }

    tuple6 -> fmatrix ( "mexpo"      , // N-tuple entry name
                        mtrx         , // matrix
                        mtrx.size()  , // number of rows (variable)
                        nCol         , // number of columns (fixed!)
                        "Len2"       ,
                        100          ) ;

  };



  { // fill with the cross-product of functionXdata vectors
    const size_t num = (size_t) poisson() ;

    typedef std::vector<double> Array ;
    Array array( num )  ;
    std::generate( array.begin () ,
                   array.end   () , flat ) ;

    typedef double (*fun)( double ) ;
    typedef std::vector<fun>  Funs  ;

    Funs funs ;
    funs.push_back( sin  ) ;
    funs.push_back( cos  ) ;
    funs.push_back( tan  ) ;
    funs.push_back( sinh ) ;
    funs.push_back( cosh ) ;
    funs.push_back( tanh ) ;

    tuple6 -> fmatrix ( "m3flat"       , // N-tuple entry name
                        funs.begin  () ,
                        funs.end    () ,
                        array.begin () ,
                        array.end   () ,
                        "Len3"         ,
                        100            ) ;

  };

  { // fill with the matrix
    const size_t num = (size_t) poisson() ;

    const size_t nCol = 15 ;

    CLHEP::HepMatrix mtrx(num,nCol) ;

    for ( size_t iRow = 0 ; iRow < num ; ++iRow )
    { for ( size_t iCol = 0 ; iCol < nCol ; ++iCol )
    { mtrx [iRow][iCol] = expo() ; } }

    tuple6 -> fmatrix ( "m2expo"       , // N-tuple entry name
                        mtrx           , // matrix
                        mtrx.num_row() , // number of rows (variable)
                        mtrx.num_col() , // number of columns (fixed!)
                        "Len4"         ,
                        100          ) ;

  };

  tuple6 -> write() ;

  // ============================================================================

  static unsigned long long evtID ( 1e14 );
  ++evtID;

  // Test for unsupported data types
  Tuple tuple7 = nTuple ( "typesCW" , "Types Test Column Wise" ) ;
  {
    tuple7 -> column ( "bool"      , (bool)               0<flat() ) ;
    tuple7 -> column ( "float"     , (float)               gauss() ) ;
    tuple7 -> column ( "double"    , (double)              gauss() ) ;
    tuple7 -> column ( "short"     , (short)               randomRange<char>() ) ;
    tuple7 -> column ( "ushort"    , (unsigned short)      randomRange<unsigned char>() ) ;
    tuple7 -> column ( "int"       , (int)                 randomRange<char>() ) ;
    tuple7 -> column ( "uint"      , (unsigned int)        randomRange<unsigned char>() ) ;
    tuple7 -> column ( "long"      , (long)                randomRange<char>() ) ;
    tuple7 -> column ( "ulong"     , (unsigned long)       randomRange<unsigned char>() ) ;
    tuple7 -> column ( "longlong"  , (long long)           randomRange<char>() ) ;
    tuple7 -> column ( "ulonglong" , (unsigned long long)  randomRange<unsigned char>() ) ;
    tuple7 -> column ( "char"      , (char)                randomRange<char>() ) ;
    tuple7 -> column ( "uchar"     , (unsigned char)       randomRange<unsigned char>() ) ;
    tuple7 -> column ( "EventID"   , evtID );
  }
  tuple7 -> write() ;


  // Test for unsupported data types
  Tuple tuple8 = nTuple ( "typesRW" , "Types Test Row Wise", CLID_RowWiseTuple ) ;
  {
    tuple8 -> column ( "bool"      , (bool)               0<flat() ) ;
    tuple8 -> column ( "float"     , (float)               gauss() ) ;
    tuple8 -> column ( "double"    , (double)              gauss() ) ;
    tuple8 -> column ( "short"     , (short)               randomRange<char>() ) ;
    tuple8 -> column ( "ushort"    , (unsigned short)      randomRange<unsigned char>() ) ;
    tuple8 -> column ( "int"       , (int)                 randomRange<char>() ) ;
    tuple8 -> column ( "uint"      , (unsigned int)        randomRange<unsigned char>() ) ;
    tuple8 -> column ( "long"      , (long)                randomRange<char>() ) ;
    tuple8 -> column ( "ulong"     , (unsigned long)       randomRange<unsigned char>() ) ;
    tuple8 -> column ( "longlong"  , (long long)           randomRange<char>() ) ;
    tuple8 -> column ( "ulonglong" , (unsigned long long)  randomRange<unsigned char>() ) ;
    tuple8 -> column ( "char"      , (char)                randomRange<char>() ) ;
    tuple8 -> column ( "uchar"     , (unsigned char)       randomRange<unsigned char>() ) ;
    tuple8 -> column ( "EventID"   , evtID );
  }
  tuple8 -> write() ;

  return StatusCode::SUCCESS ;
}

// ============================================================================
DECLARE_COMPONENT(TupleAlg)
// ============================================================================

// ============================================================================
// The END
// ============================================================================
