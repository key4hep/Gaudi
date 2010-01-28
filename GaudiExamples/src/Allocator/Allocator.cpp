// $Id: Allocator.cpp,v 1.2 2006/11/27 09:53:05 hmd Exp $
// ============================================================================
// CVS tag $Name:  $ 
// ============================================================================
// Include file
// ============================================================================
// STD & STL 
// ============================================================================
#include <cmath>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
// ============================================================================
// Boost
// ============================================================================
#include "boost/format.hpp"
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ChronoEntity.h"
#include "GaudiKernel/System.h"
// ============================================================================
// Local
// ============================================================================
#include "MyClass1.h"
#include "MyClass1A.h"
// ============================================================================

/** @file 
 *  Test for the class GaudiUtils::Allocator 
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2006-02-10
 */

namespace 
{
  
  std::ostream& operator<< 
    ( std::ostream&       stream ,
      const ChronoEntity& ent    )
  {
    return stream 
      << "U:"   << ent.outputUserTime    () 
      << "\nS:" << ent.outputSystemTime  () 
      << "\nE:" << ent.outputElapsedTime () ;
//     return stream 
//       << "K:t/m/em/RMS:"
//       << "   " << ent.kTotalTime     () 
//       << " \t" << ent.kMeanTime      () 
//       << " \t" << ent.kMeanErrorTime () 
//       << " \t" << ent.kRMSTime       () 
//       << std::endl 
//       << "U:t/m/em/RMS:"
//       << "   " << ent.uTotalTime     () 
//       << " \t" << ent.uMeanTime      () 
//       << " \t" << ent.uMeanErrorTime () 
//       << " \t" << ent.uRMSTime       () 
//       << std::endl 
//       << "E:t/m/em/RMS:"
//       << "   " << ent.eTotalTime     () 
//       << " \t" << ent.eMeanTime      () 
//       << " \t" << ent.eMeanErrorTime () 
//       << " \t" << ent.eRMSTime       () ;
  }
}


namespace 
{
  /** the simple function to test the preformace of 
   *  allocators. Actually it tests the CPU 
   *  performace of operators "new" and "delete"
   *
   *  The test is just trivial: 
   *
   *   - "number" of objects are created and 
   *      deleted in a random order 
   *   - the test is repeated "repeate" times 
   *
   *  Only the creation and deletion times 
   *  are accounted 
   *
   *  @param number  nnmber of object to be created/deleted 
   *  @param repeat  number of repetitions 
   *  @param chrono  CPU=measurer
   */
  template <class TYPE> 
  void makeTest  
  ( const size_t  number , 
    const size_t  repeat , 
    ChronoEntity& chrono ) 
  {
    
    // create an vector of pointers (FIXED length!)
    typedef std::vector<TYPE*> Vct ;    
    Vct vct = Vct ( number , (TYPE*)0 ) ;
    
    // sturt the loop over repetition
    for ( size_t irep = 0 ; irep < repeat ; ++irep ) 
    { 
      
      { // A) create objects
        // - 1) start the clock
        chrono.start()  ;
        // - 2) fill the vector with the data
        for (  size_t i = 0 ; i < number ; ++i ) { vct[i]= new TYPE() ; }
        // - 3)stop the clock
        chrono.stop() ;
      }
      
      { // B) reshuffle the vector to ensure the 
        //    random order of destruction
        std::random_shuffle( vct.begin() , vct.end() ) ;
      }
      
      { // C) delete objects
        // - 1) start clock 
        chrono.start() ;
        // - 2) delete the content 
        for (  size_t i = 0 ; i < number ; ++i ) { delete vct[i] ; }
        // - 3) stop clock 
        chrono.stop() ;
      }
      
    } // end of repetition loop
  }
}

int main()
{
  
  const size_t number  = 10000 ;
  const size_t repeate =  4000 ;
  
  // normalization test with "char"
  {
    typedef char Test ;
    
    ChronoEntity test ;
    
    std::cout << " Tests with "
              << System::typeinfoName( typeid( Test) ) 
              << " size=" << sizeof( Test ) << std::endl ;
    makeTest<Test> ( number , repeate , test ) ;
    std::cout << test << std::endl ;
    
  } ;

  // class without allocator 
  {
    typedef MyClass1 Test ;
    
    ChronoEntity test ;
    
    std::cout << " Tests with "
              << System::typeinfoName( typeid( Test) ) 
              << " size=" << sizeof( Test ) << std::endl ;
    makeTest<Test> ( number , repeate , test ) ;
    std::cout << test << std::endl ;
    
  } ;
  
  // class with allocator 
  {
    typedef MyClass1A Test ;
    
    ChronoEntity test ;
    
    std::cout << " Tests with "
              << System::typeinfoName( typeid( Test) ) 
              << " size=" << sizeof( Test ) << std::endl ;
    makeTest<Test> ( number , repeate , test ) ;
    std::cout << test << std::endl ;
    
  } ;

  // class without allocator 
  {
    typedef MyClass1 Test ;
    
    ChronoEntity test ;
    
    std::cout << " Tests with "
              << System::typeinfoName( typeid( Test) ) 
              << " size=" << sizeof( Test ) << std::endl ;
    makeTest<Test> ( number , repeate , test ) ;
    std::cout << test << std::endl ;
    
  } ;

  // class with allocator 
  {
    typedef MyClass1A Test ;
    
    ChronoEntity test ;
    
    std::cout << " Tests with "
              << System::typeinfoName( typeid( Test) ) 
              << " size=" << sizeof( Test ) << std::endl ;
    makeTest<Test> ( number , repeate , test ) ;
    std::cout << test << std::endl ;
    
  } ;

  
  return 0 ;
}


// ============================================================================
// The END 
// ============================================================================
