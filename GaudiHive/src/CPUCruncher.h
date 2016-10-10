
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/RegistryEntry.h"

#include <tbb/concurrent_hash_map.h>

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

  public:

    typedef tbb::concurrent_hash_map<std::string,unsigned int> CHM;

    /// the execution of the algorithm
    StatusCode execute  () override ; // the execution of the algorithm
    /// Its initialization
    StatusCode initialize() override;
    /// the finalization of the algorithm
    StatusCode finalize () override ; // the finalization of the algorithm

    double get_runtime() const { return m_avg_runtime; };

    CPUCruncher
    ( const std::string& name , // the algorithm instance name
      ISvcLocator*       pSvc ); // the Service Locator

    /// virtual & protected desctrustor
    ~CPUCruncher() override;     // virtual & protected desctrustor

  private:

    /// the default constructor is disabled
    CPUCruncher () ;                              // no default constructor
    /// the copy constructor is disabled
    CPUCruncher            ( const CPUCruncher& ) ; // no copy constructor
    /// the assignement operator is disabled
    CPUCruncher& operator= ( const CPUCruncher& ) ; // no assignement
    /// The CPU intensive function
    void findPrimes (const unsigned long int ) ;

    /// Calibrate
    void calibrate();
    long unsigned int getNCaliIters(double);

    double m_avg_runtime ; //Avg Runtime
    double m_var_runtime ; //Variance of Runtime
    bool m_local_rndm_gen; //Decide what random number generation to use
    bool m_shortCalib;

    // To calib only once
    static std::vector<unsigned int>m_niters_vect;
    static std::vector<double> m_times_vect;

    // For the concurrency

    const uint MAX_INPUTS = 40;
    const uint MAX_OUTPUTS = 10;

    std::vector<DataObjectHandle<DataObject> *> m_inputHandles;
    std::vector<DataObjectHandle<DataObject> *> m_outputHandles;

    std::vector<std::string> m_inpKeys, m_outKeys;

    unsigned int m_rwRepetitions;

    static CHM m_name_ncopies_map;

    // Fraction of total execution time, during which an algorithm is actually sleeping instead of crunching CPU
    float m_sleepFraction;
  };

