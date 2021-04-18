/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/ICPUCrunchSvc.h"
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
class CPUCruncher : public GaudiAlgorithm {

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
               ISvcLocator*       pSvc );     // the Service Locator

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

  /// Pick up late-attributed data outputs
  void                  declareRuntimeRequestedOutputs();
  bool                  m_declAugmented{false};
  Gaudi::Property<bool> m_loader{this, "Loader", false, "Declare the algorithm to be a data loader"};

  Gaudi::Property<std::vector<std::string>> m_inpKeys{this, "inpKeys", {}, ""};
  Gaudi::Property<std::vector<std::string>> m_outKeys{this, "outKeys", {}, ""};

  Gaudi::Property<double> m_avg_runtime{this, "avgRuntime", 1., "Average runtime of the module."};
  Gaudi::Property<double> m_var_runtime{this, "varRuntime", 0.01, "Variance of the runtime of the module."};
  Gaudi::Property<bool> m_local_rndm_gen{this, "localRndm", true, "Decide if the local random generator is to be used"};
  Gaudi::Property<unsigned int> m_rwRepetitions{this, "RwRepetitions", 1, "Increase access to the WB"};
  Gaudi::Property<float>        m_sleepFraction{
      this, "SleepFraction", 0.0f,
      "Fraction of time, between 0 and 1, when an algorithm is actually sleeping instead of crunching"};
  Gaudi::Property<bool>         m_invertCFD{this, "InvertDecision", false, "Invert control flow decision."};
  Gaudi::Property<unsigned int> m_failNEvents{this, "FailNEvents", 0, "Return FAILURE on every Nth event"};
  Gaudi::Property<int>          m_nParallel{this, "NParallel", -1, "Run N parallel crunching tasks"};

  // For the concurrency
  const uint MAX_INPUTS  = 40;
  const uint MAX_OUTPUTS = 10;

  std::vector<DataObjectHandle<DataObject>*> m_inputHandles;
  std::vector<DataObjectHandle<DataObject>*> m_outputHandles;

  static CHM m_name_ncopies_map;

  // CPUCrunchSvc
  SmartIF<ICPUCrunchSvc> m_crunchSvc;
};
