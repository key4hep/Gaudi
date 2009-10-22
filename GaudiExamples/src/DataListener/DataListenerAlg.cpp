// $Id: DataListenerAlg.cpp,v 1.1 2006/09/04 08:22:30 hmd Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h" 

// local
#include "DataListenerAlg.h"
#include <cmath>

//-----------------------------------------------------------------------------
// Implementation file for class : DataListenerAlg
//
// 2006-08-28 : Ben King
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( DataListenerAlg );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
DataListenerAlg::DataListenerAlg( const std::string& name,
            ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
    , m_fibo(1)
    , m_geo(1)
    , m_arith(1)
    , m_expo(1.01)
{
}
//=============================================================================
// Destructor
//=============================================================================
DataListenerAlg::~DataListenerAlg() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode DataListenerAlg::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first

  
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  try {
    sc = service("DataListenerSvc", m_DataListenerSvc, true);

    m_DataListenerSvc->declareInfo("Fibonacci number", m_fibo, "Fibonacci series", this);
    m_DataListenerSvc->declareInfo("Exponential number", m_expo, "Exponential series", this);
    m_DataListenerSvc->declareInfo("Arithmetic number", m_arith, "Arithmetic series", this);
    m_DataListenerSvc->declareInfo("Geometric number", m_geo, "Geometric series", this);


  } catch (std::runtime_error &e){
    info() << "WARNING loading DataListenerSvc:\t" << e.what() << endmsg;
  }  


    debug() << "==> Initialize" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode DataListenerAlg::execute() {

  debug() << "==> Execution counter:\t" << endmsg;

//   StatusCode sc = Arith(m_arith, 2, 1);
//   sc = Expo(m_expo, 1);
//   sc = Fibonacci(m_fibo, 1, 1);
//   sc = Geo(m_geo, 2, 1);
//   m_counter++;

  StatusCode sc = Arith( 2, 1);
  sc = Expo( 1);
  sc = Fibonacci( 1, 1);
  sc = Geo( 2, 1);
  m_counter++;

  
  return sc;

}



// StatusCode DataListenerAlg::Arith(int& startPoint, int difference, unsigned int numSteps) 
// {

//   startPoint += difference*numSteps;
  

//   setFilterPassed(true);   // Mandatory. Set to true if event is accepted.
//   return StatusCode::SUCCESS;

  
// }

StatusCode DataListenerAlg::Arith(int difference, unsigned int numSteps) 
{

  m_arith += difference*numSteps;
  

  return StatusCode::SUCCESS;

  
}



// StatusCode DataListenerAlg::Expo(double& startPoint, unsigned int numSteps) 
// {

//   for(unsigned int i=0; i<numSteps; i++){
//     startPoint = startPoint*startPoint;
//   }
  

//   setFilterPassed(true);   // Mandatory. Set to true if event is accepted.
//   return StatusCode::SUCCESS;
// }


StatusCode DataListenerAlg::Expo(unsigned int numSteps) 
{

  for(unsigned int i=0; i<numSteps; i++){
    m_expo = m_expo*m_expo;
  }
  

  return StatusCode::SUCCESS;
}


// StatusCode DataListenerAlg::Fibonacci(int& startPoint1, int startPoint2, unsigned int numSteps) 
// {
//   int temp1;

//   for (unsigned int i = 0; i < numSteps; i++){
//     temp1 = startPoint1 + startPoint2;
//     startPoint1 = startPoint2;
//     startPoint2 = temp1;
//   }
  

//   setFilterPassed(true);   // Mandatory. Set to true if event is accepted.
//   return StatusCode::SUCCESS;
  

// }

StatusCode DataListenerAlg::Fibonacci(int startPoint2, unsigned int numSteps) 
{
  int temp1;

  for (unsigned int i = 0; i < numSteps; i++){
    temp1 = m_fibo + startPoint2;
    m_fibo = startPoint2;
    startPoint2 = temp1;
  }
  

  return StatusCode::SUCCESS;
  

}




// StatusCode DataListenerAlg::Geo(int& startPoint, int ratio, unsigned int numSteps) 
// {

//   for (unsigned int i = 0; i< numSteps; i++){
//     startPoint = startPoint*ratio;
//   }
//   setFilterPassed(true);   // Mandatory. Set to true if event is accepted.
//   return StatusCode::SUCCESS;
  
// }


StatusCode DataListenerAlg::Geo(int ratio, unsigned int numSteps) 
{

  for (unsigned int i = 0; i< numSteps; i++){
    m_geo = m_geo*ratio;
  }

  return StatusCode::SUCCESS;
  
}




    
    
    



//=============================================================================
//  Finalize
//=============================================================================
StatusCode DataListenerAlg::finalize() {

  debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================
