#include <ctime>
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiAlg/GaudiAlgorithm.h"

//------------------------------------------------------------------------------

  /** @class CPUCruncher
   * 
   * A class that implements a search for prime numbers.
   * A lot of allocations are performed on purpose not to stress only the FP 
   * unit of the CPU.
   * 
   */
  class CPUCruncher : public GaudiAlgorithm 
  {

    friend class AlgFactory<CPUCruncher> ;

  public:

    /// the execution of the algorithm 
    virtual StatusCode execute  () ; // the execution of the algorithm 
    /// the finalization of the algorithm 
    virtual StatusCode finalize () ; // the finalization of the algorithm 

  protected:

    CPUCruncher 
    ( const std::string& name , // the algorithm instance name 
      ISvcLocator*       pSvc ); // the Service Locator 

    /// virtual & protected desctrustor 
    virtual ~CPUCruncher() {}     // virtual & protected desctrustor

  private:

    /// the default constructor is disabled 
    CPUCruncher () ;                              // no default constructor
    /// the copy constructor is disabled
    CPUCruncher            ( const CPUCruncher& ) ; // no copy constructor 
    /// the assignement operator is disabled
    CPUCruncher& operator= ( const CPUCruncher& ) ; // no assignement
    /// The CPU intensive function
    void findPrimes (const double runtime) ; 

    double m_avg_runtime ; //Avg Runtime
    double m_var_runtime ; //Variance of Runtime
  };
