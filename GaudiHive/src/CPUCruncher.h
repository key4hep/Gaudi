
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
    virtual StatusCode execute  () ; // the execution of the algorithm
    /// Its initialization
    virtual StatusCode initialize();
    /// the finalization of the algorithm
    virtual StatusCode finalize () ; // the finalization of the algorithm

    double get_runtime() const { return m_avg_runtime; };

    CPUCruncher
    ( const std::string& name , // the algorithm instance name
      ISvcLocator*       pSvc ); // the Service Locator

    /// virtual & protected desctrustor
    virtual ~CPUCruncher();     // virtual & protected desctrustor

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


    StringArrayProperty  m_inpKeys {this, "inpKeys",  {}, ""};
    StringArrayProperty  m_outKeys {this, "outKeys",  {}, ""};

    DoubleProperty  m_avg_runtime {this, "avgRuntime",  1.,  "Average runtime of the module."};
    DoubleProperty  m_var_runtime {this, "varRuntime",  0.01,  "Variance of the runtime of the module."};
    BooleanProperty  m_local_rndm_gen {this, "localRndm",  true,  "Decide if the local random generator is to be used"};
    BooleanProperty  m_shortCalib {this, "shortCalib",  false,  "Enable coarse grained calibration"};
    UnsignedIntegerProperty  m_rwRepetitions {this, "RwRepetitions",  1,  "Increase access to the WB"};
    BooleanProperty  m_sleepyExecution {this, "SleepyExecution",  false,  "Sleep during execution instead of crunching"};

    // To calib only once
    static std::vector<unsigned int> m_niters_vect;
    static std::vector<double> m_times_vect;

    // For the concurrency
    const uint MAX_INPUTS = 40;
    const uint MAX_OUTPUTS = 10;

    std::vector<DataObjectHandle<DataObject> *> m_inputHandles;
    std::vector<DataObjectHandle<DataObject> *> m_outputHandles;

    static CHM m_name_ncopies_map;
  };
