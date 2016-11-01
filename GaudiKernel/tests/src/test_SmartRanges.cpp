// ============================================================================
// Include files/
// ============================================================================
// STD & STL
// ============================================================================
#include <iostream>
#include <algorithm>
#include <vector>
#include <functional>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ToStream.h"
#include "GaudiKernel/SmartRanges.h"
// ============================================================================
// Boost 
// ============================================================================
#include "boost/dynamic_bitset.hpp"
#include "boost/test/minimal.hpp"
//#include <boost/test/unit_test.hpp>
// ============================================================================
/** @file
 *  Test-file for "smart-ranges" 
 *  @see Gaudi::Range_
 *  @see Gaudi::make_filter_range 
 *  @see Gaudi::make_transform_range 
 *  @see Gaudi::make_mask_range 
 *  @see Gaudi::make_index_range 
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date 2016-09-20
 */
// ============================================================================
struct TEST2 { bool operator() ( int x ) { return x%2 != 0 ;} } ;
struct TEST3 { bool operator() ( int x ) { return x%3 != 0 ;} } ;
// struct TEST5 { bool operator() ( int x ) { return x%5 != 0 ;} } ;
// ============================================================================  
///
/// BOOST_AUTO_TEST_CASE(test_main)
/// {
///   ...
/// }
/// @endcode
//BOOST_AUTO_TEST_CASE(test_main)
int test_main ( int /*argc*/, char** /*argv*/)             // note the name!
{
  
  // input data
  typedef std::vector<int>  DATA ;
  DATA v(30) ;
  std::iota ( v.begin() , v.end() , 0 ) ;
  //

  // 
  // Test transforms(&filters)
  //                                                                    
  
  std::cout << "*****************************************************" << std::endl ;
  std::cout << "START with        : ";
  Gaudi::Utils::toStream ( v , std::cout ) << std::endl ;
  
  auto r  = Gaudi::make_filter_range    ( [](int i)->bool{ return i%3 != 0; }  , v ) ;
  std::cout << "FILTER x%3!=0     : " ;
  Gaudi::Utils::toStream ( r.begin() , r.end() , std::cout , "[" , "]" , " , ") << std::endl ;
  BOOST_CHECK( 20 == r.size() ) ;
  BOOST_CHECK(  1 == r[0] && 2 == r[1] && 4 == r[2] && 5 ==r[3] ) ;
  
  auto r1 = Gaudi::make_transform_range ( [](int i)->int { return i+2; } , r ) ;
  std::cout << "TRANSFORM x+2     : " ;
  Gaudi::Utils::toStream ( r1.begin() , r1.end() , std::cout , "[" , "]" , " , ") << std::endl ;
  BOOST_CHECK( 20 == r1.size() ) ;
  BOOST_CHECK(  3 == r1[0] && 4 == r1[1] && 6 == r1[2] && 7 ==r1[3] ) ;
  
  auto r2 = Gaudi::make_transform_range ( [](int i)->int { return i*i ; } , r1 ) ;
  std::cout << "TRANSFORM x*x     : " ;
  Gaudi::Utils::toStream ( r2.begin() , r2.end() , std::cout , "[" , "]" , " , ") << std::endl ;
  BOOST_CHECK( 20 == r2.size() ) ;
  BOOST_CHECK(  9 == r2[0] && 16 == r2[1] && 36 == r2[2] && 49 == r2[3] ) ;
  
  auto r3 = Gaudi::make_transform_range ( [](int i)->int { return i%10 ; } , r2 ) ;
  std::cout << "TRANSFORM x%10    : " ;
  Gaudi::Utils::toStream ( r3.begin() , r3.end() , std::cout , "[" , "]" , " , ") << std::endl ;
  BOOST_CHECK( 20 == r3.size() ) ;
  BOOST_CHECK(  9 == r3[0] && 6 == r3[1] && 6 == r3[2] && 9 == r3[3] ) ;
  
  auto r4 = Gaudi::make_transform_range ( [](int i)->int { return i*i ; } , r3 ) ;
  std::cout << "TRANSFORM x*x     : " ;
  Gaudi::Utils::toStream ( r4.begin() , r4.end() , std::cout , "[" , "]" , " , ") << std::endl ;
  BOOST_CHECK( 20 == r4.size() ) ;
  BOOST_CHECK(81 == r4[0] && 36 == r4[1] && 36 == r4[2] && 81 == r4[3] ) ;

  //
  // Test filters 
  // 
  
  std::cout << "*****************************************************" << std::endl ;
  
  std::cout << "(RE)START with    : " ;
  Gaudi::Utils::toStream ( v.begin() , v.end() , std::cout , "[" , "]" , " , ") << std::endl ;


  auto v0  = Gaudi::make_filter_range    ( TEST2()   , v ) ;
  std::cout << "FILTER x%2!=0     : " ;
  Gaudi::Utils::toStream ( v0.begin() , v0.end() , std::cout , "[" , "]" , " , ") << std::endl ;
  BOOST_CHECK( 15 == v0.size() ) ;
  BOOST_CHECK(  1 == v0[0] && 3 == v0[1] && 5 == v0[2] && 7 == v0[3] ) ;

  // auto v1  = Gaudi::make_filter_range    ( [](int i)->bool{ return i%3 != 0; }    , v0) ;
  auto v1  = make_filter_range    ( TEST3()  , v0) ;
  std::cout << "FILTER x%3!=0     : " ;
  Gaudi::Utils::toStream ( v1.begin() , v1.end() , std::cout , "[" , "]" , " , ") << std::endl ;
  BOOST_CHECK( 10 == v1.size() ) ;
  BOOST_CHECK(  1 == v1[0] && 5 == v1[1] && 7 == v1[2] && 11 == v1[3] ) ;
  
  auto v2  = Gaudi::make_filter_range    ( [](int i)->bool{ return i%5 != 0; }    , v1) ;
  // auto v2  = make_filter_range    ( TEST5()  , v1) ;
  std::cout << "FILTER x%5!=0     : " ;
  Gaudi::Utils::toStream ( v2.begin() , v2.end() , std::cout , "[" , "]" , " , ") << std::endl ;
  BOOST_CHECK(  8 == v2.size() ) ;
  BOOST_CHECK(  1 == v2[0] && 7 == v2[1] && 11 == v2[2] && 13 == v2[3] ) ;
  
  //
  // Test masks
  //
  
  std::cout << "*****************************************************" << std::endl ;
  std::cout << "(RE)START with    : " ;
  Gaudi::Utils::toStream ( v.begin() , v.end() , std::cout , "[" , "]" , " , ") << std::endl ;
  
  using mask_t = boost::dynamic_bitset<unsigned long> ;
  
  mask_t mask (100) ;
  mask.set( 4,true) ;
  mask.set( 5,true) ;
  mask.set(10,true) ;
  mask.set(15,true) ;
  mask.set(50,true) ;
  
  
  auto z1 = Gaudi::make_mask_range ( mask , v ) ;
  std::cout << "MASK(4,5,10,15,50): " ;
  Gaudi::Utils::toStream ( z1.begin() , z1.end() , std::cout , "[" , "]" , " , ") << std::endl ;
  BOOST_CHECK(  4 == z1.size() ) ;
  BOOST_CHECK(  4 == z1[0] && 5 == z1[1] && 10 == z1[2] && 15 == z1[3] ) ;

  auto z2 = Gaudi::make_transform_range ( [](int i)->int { return i/5+1; } , z1 ) ;
  std::cout << "TRANSFORM x/5+1   : " ;
  Gaudi::Utils::toStream ( z2.begin() , z2.end() , std::cout , "[" , "]" , " , ") << std::endl ;
  BOOST_CHECK(  4 == z2.size() ) ;
  BOOST_CHECK(  1 == z2[0] && 2 == z2[1] && 3 == z2[2] && 4 == z2[3] ) ;


  //
  // Test indices 
  //
  
  std::cout << "*****************************************************" << std::endl ;
  std::cout << "(RE)START with    : " ;
  Gaudi::Utils::toStream ( v.begin() , v.end() , std::cout , "[" , "]" , " , ") << std::endl ;
  
  typedef std::vector<unsigned long> IX;
  const IX ix = { 3 , 3 , 5 , 5 , 9 } ; // note repetition  
  
  auto x1 = Gaudi::make_index_range ( ix , v ) ;
  std::cout << "INDICES(3,3,5,5,9): " ;
  Gaudi::Utils::toStream ( x1.begin() , x1.end() , std::cout , "[" , "]" , " , ") << std::endl ;
  BOOST_CHECK(  5 == x1.size() ) ;
  BOOST_CHECK(  3 == x1[0] && 3 == x1[1] && 5 == x1[2] && 5 == x1[3] ) ;

  auto x2  = Gaudi::make_filter_range    ( [](int i)->bool{ return i%3 != 0; }    , x1) ;
  std::cout << "FILTER x%3!=0     : " ;
  Gaudi::Utils::toStream ( x2.begin() , x2.end() , std::cout , "[" , "]" , " , ") << std::endl ;
  BOOST_CHECK(  2 == x2.size() ) ;
  BOOST_CHECK(  5 == x2[0] && 5 == x2[1] ) ;
  
  return 0 ;
}
//=============================================================================
