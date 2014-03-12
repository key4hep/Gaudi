// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <utility>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/SystemOfUnits.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/Tuple.h"
#include "GaudiAlg/GaudiTupleAlg.h"
// ============================================================================

// ============================================================================
/** @class TupleAlg3
 *
 *  Example of usage of specilized columns for
 *  some ROOT objects (math-, geometry-&kinematics-vectors and matrices)
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2006-11-26
 */
// ============================================================================


class TupleAlg3 : public GaudiTupleAlg
{
public:
  /// initialize the algorithm
  StatusCode initialize ()
  {
    StatusCode sc = GaudiTupleAlg::initialize() ;
    if ( sc.isFailure() ) { return sc ; }
    // check for random numbers service
    Assert ( randSvc() != 0 , "Random Service is not available!");
    //
    return StatusCode::SUCCESS ;
  };
  /** the only one essential method
   *  @see IAlgoruthm
   */
  StatusCode execute    () ;
  /** standard constructor
   *  @param name algorithm instance name
   *  @param pSvc pointer to Service Locator
   */
  TupleAlg3
  ( const std::string& name ,
    ISvcLocator*       pSvc )
    : GaudiTupleAlg ( name , pSvc ) {};
  // destructor
  virtual ~TupleAlg3 () {} ;
private:
  // default constructor is disabled
  TupleAlg3() ;
  // copy constructor is disabled
  TupleAlg3( const TupleAlg3& ) ;
  // assignement op[erator is disabled
  TupleAlg3& operator=( const TupleAlg3& ) ;
};
// ============================================================================
DECLARE_COMPONENT(TupleAlg3)
// ============================================================================
/** the only one essential method
 *  @see IAlgoruthm
 */
// ============================================================================
StatusCode TupleAlg3::execute()
{
  /// avoid long names
  using namespace Tuples       ;
  using namespace Gaudi::Units ;

  Rndm::Numbers gauss   ( randSvc() , Rndm::Gauss       (   0.0 ,  1.0 ) ) ;
  Rndm::Numbers flat    ( randSvc() , Rndm::Flat        ( -10.0 , 10.0 ) ) ;
  Rndm::Numbers expo    ( randSvc() , Rndm::Exponential (   1.0        ) ) ;
  Rndm::Numbers breit   ( randSvc() , Rndm::BreitWigner (   0.0 ,  1.0 ) ) ;
  Rndm::Numbers poisson ( randSvc() , Rndm::Poisson     (   2.0        ) ) ;
  Rndm::Numbers binom   ( randSvc() , Rndm::Binomial    (   8   , 0.25 ) ) ;


  // ==========================================================================
  // book and fill simple NTuple with Lorentz Vectors
  // ==========================================================================
  {
    Tuple tuple = nTuple ( "LorentzVectors" , "Tuple with Lorentz Vectors " ) ;

    typedef ROOT::Math::PxPyPzEVector     LV;

    LV lv  ;

    lv.SetPx ( gauss () ) ;
    lv.SetPy ( gauss () ) ;
    lv.SetPz ( flat  () ) ;
    lv.SetE  ( ::sqrt( lv.P2() +  1 * GeV * GeV ) ) ;

    LV lv2 = lv ;
    lv2.SetE ( ::sqrt( lv.P2() +  4 * GeV * GeV ) ) ;

    tuple -> column("lv1" , lv  ) ;
    tuple -> column("lv2" , lv2 ) ;

    tuple -> write() ;
  }
  // ==========================================================================
  // book and fill simple NTuple with 3D Vectors & Points
  // ==========================================================================
  {

    Tuple tuple = nTuple ( "Vectors-3D" , "Tuple with Vectors in 3D " ) ;

    typedef ROOT::Math::XYZPoint  P3;
    typedef ROOT::Math::XYZVector V3;

    P3 p1 ;
    p1.SetX ( flat() ) ;
    p1.SetY ( flat() ) ;
    p1.SetZ ( flat() ) ;

    tuple -> column ( "p3" , p1 ) ;

    V3 p2 ;
    p2.SetX ( gauss () ) ;
    p2.SetY ( gauss () ) ;
    p2.SetZ ( gauss () ) ;

    tuple -> column ( "v3" , p2 ) ;

    tuple -> write() ;
  }
  // ==========================================================================
  // book and fill simple NTuple with Math-vectors
  // ==========================================================================
  {

    Tuple tuple = nTuple ( "Math-vectors" , "Tuple with Math Vectors" ) ;

    ROOT::Math::SVector<float,4>          v1 ;
    ROOT::Math::SVector<double,25>        v2 ;
    ROOT::Math::SVector<long double,50>   v3 ;

    std::generate( v1.begin() , v1.end() , gauss ) ;
    std::generate( v2.begin() , v2.end() , breit ) ;
    std::generate( v3.begin() , v3.end() , flat  ) ;

    tuple -> array ( "g" , v1  ) ;
    tuple -> array ( "b" , v2  ) ;
    tuple -> array ( "f" , v3  ) ;

    tuple -> write() ;
  }
  // ==========================================================================
  // book and fill simple NTuple with S-matrices
  // ==========================================================================
  {

    Tuple tuple = nTuple ( "S-matrices" , "Tuple with S-matrices" ) ;

    ROOT::Math::SMatrix<float,4>          v1 ;
    ROOT::Math::SMatrix<double,3,15>      v2 ;
    ROOT::Math::SMatrix<long double,5,5,ROOT::Math::MatRepSym<long double,5> >   v3 ;

    std::generate( v1.begin() , v1.end() , gauss ) ;
    std::generate( v2.begin() , v2.end() , breit ) ;
    std::generate( v3.begin() , v3.end() , flat  ) ;

    tuple -> matrix ( "g" , v1  ) ;
    tuple -> matrix ( "b" , v2  ) ;
    tuple -> matrix ( "f" , v3  ) ;

    tuple -> write() ;
  }

  // ==========================================================================
  // book and fill simple NTuple with VectorMap
  // ==========================================================================
  {

    Tuple tuple = nTuple ( "VectorMaps" , "Tuple with VectorMaps/'ExtraInfo'" ) ;

    typedef  GaudiUtils::VectorMap<int,double>       VM1 ;
    typedef  GaudiUtils::VectorMap<char,long double> VM2 ;
    typedef  GaudiUtils::VectorMap<int,float>        VM3 ;
    typedef  GaudiUtils::VectorMap<long,double>      VM4 ;

    VM1 vm1 ;
    VM2 vm2 ;
    VM3 vm3 ;
    VM4 vm4 ;

    for ( int i = 200 ; 0 < i; --i )
    {
      vm1.insert ( std::make_pair ( i , i * 2 ) ) ;
      vm2.insert ( std::make_pair ( i , i * 3 ) ) ;
      vm3.insert ( std::make_pair ( i , float(i * 4) ) ) ;
      vm4.insert ( std::make_pair ( i , i * 5 ) ) ;
    }

    tuple -> fmatrix ( "map1" , vm1 , "n1" , 500 ) ;
    tuple -> fmatrix ( "map2" , vm2 , "n2" , 500 ) ;
    tuple -> fmatrix ( "map3" , vm3 , "n3" , 500 ) ;
    tuple -> fmatrix ( "map4" , vm4 , "n4" , 500 ) ;

    tuple->write() ;

  }

  return StatusCode::SUCCESS ;

}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
