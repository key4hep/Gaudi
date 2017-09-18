
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RegistryEntry.h"
#include "GaudiKernel/RndmGenerators.h"

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
  typedef tbb::concurrent_hash_map<std::string, unsigned int> CHM;

  bool isClonable() const override { return true; }

  /// the execution of the algorithm
  StatusCode execute() override;
  /// Its initialization
  StatusCode initialize() override;
  /// the finalization of the algorithm
  StatusCode finalize() override;

  double get_runtime() const { return m_avg_runtime; }

  CPUCruncher( const std::string& name, // the algorithm instance name
               ISvcLocator* pSvc );     // the Service Locator

  /// virtual & protected desctrustor
  virtual ~CPUCruncher(); // virtual & protected desctrustor

private:
  /// the default constructor is disabled
  CPUCruncher(); // no default constructor
  /// the copy constructor is disabled
  CPUCruncher( const CPUCruncher& ); // no copy constructor
  /// the assignement operator is disabled
  CPUCruncher& operator=( const CPUCruncher& ); // no assignement
  /// The CPU intensive function
  void findPrimes( const unsigned long int );

  /// Calibrate
  void calibrate();
  long unsigned int getNCaliIters( double );

  Gaudi::Property<std::vector<std::string>> m_inpKeys{this, "inpKeys", {}, ""};
  Gaudi::Property<std::vector<std::string>> m_outKeys{this, "outKeys", {}, ""};

  Gaudi::Property<double> m_avg_runtime{this, "avgRuntime", 1., "Average runtime of the module."};
  Gaudi::Property<double> m_var_runtime{this, "varRuntime", 0.01, "Variance of the runtime of the module."};
  Gaudi::Property<bool> m_local_rndm_gen{this, "localRndm", true, "Decide if the local random generator is to be used"};
  Gaudi::Property<bool> m_shortCalib{this, "shortCalib", false, "Enable coarse grained calibration"};
  Gaudi::Property<unsigned int> m_rwRepetitions{this, "RwRepetitions", 1, "Increase access to the WB"};
  Gaudi::Property<float> m_sleepFraction{
      this, "SleepFraction", 0.0f,
      "Fraction of time, between 0 and 1, when an algorithm is actually sleeping instead of crunching"};
  Gaudi::Property<bool> m_invertCFD{this, "InvertDecision", false, "Invert control flow decision."};

  // To calib only once
  static std::vector<unsigned int> m_niters_vect;
  static std::vector<double> m_times_vect;

  // For the concurrency
  const uint MAX_INPUTS  = 40;
  const uint MAX_OUTPUTS = 10;

  std::vector<DataObjectHandle<DataObject>*> m_inputHandles;
  std::vector<DataObjectHandle<DataObject>*> m_outputHandles;

  static CHM m_name_ncopies_map;
};
