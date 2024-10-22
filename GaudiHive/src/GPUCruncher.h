/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include <Gaudi/AsynchronousAlgorithm.h>
#include <GaudiKernel/DataObjectHandle.h>
#include <GaudiKernel/IRndmGenSvc.h>
#include <GaudiKernel/RegistryEntry.h>
#include <GaudiKernel/RndmGenerators.h>
#include <tbb/concurrent_hash_map.h>

//------------------------------------------------------------------------------

/** @class GPUCruncher
 *
 *  A test asynchronous algorithm. Might eventually run computations on a GPU but for now
 *  it just sleeps for a few seconds.
 */
class GPUCruncher : virtual public Gaudi::AsynchronousAlgorithm {

public:
  typedef tbb::concurrent_hash_map<std::string, unsigned int> CHM;

  bool isClonable() const override { return true; }

  /// the execution of the algorithm
  StatusCode execute( const EventContext& ctx ) const override;
  /// Its initialization
  StatusCode initialize() override;
  /// the finalization of the algorithm
  StatusCode finalize() override;

  double get_runtime() const { return m_avg_runtime; }

  GPUCruncher( const std::string& name, // the algorithm instance name
               ISvcLocator*       pSvc );     // the Service Locator

  /// virtual & protected desctrustor
  virtual ~GPUCruncher(); // virtual & protected destructor

private:
  /// the default constructor is disabled
  GPUCruncher(); // no default constructor
  /// the copy constructor is disabled
  GPUCruncher( const GPUCruncher& ); // no copy constructor
  /// the assignement operator is disabled
  GPUCruncher& operator=( const GPUCruncher& ); // no assignement
  /// The GPU intensive function
  StatusCode gpuExecute( const std::pmr::vector<double>& in, std::vector<double>& out ) const;

  Gaudi::Property<std::vector<std::string>> m_inpKeys{ this, "inpKeys", {}, "" };
  Gaudi::Property<std::vector<std::string>> m_outKeys{ this, "outKeys", {}, "" };

  Gaudi::Property<double> m_avg_runtime{ this, "avgRuntime", 1., "Average runtime of the module." };
  Gaudi::Property<double> m_var_runtime{ this, "varRuntime", 0.01, "Variance of the runtime of the module." };
  Gaudi::Property<bool>   m_local_rndm_gen{ this, "localRndm", true,
                                          "Decide if the local random generator is to be used" };

  // For allocating CUDA pinned memory
  std::pmr::memory_resource* pinned;

  // For the concurrency
  const uint MAX_INPUTS  = 40;
  const uint MAX_OUTPUTS = 10;

  std::vector<DataObjectHandle<DataObject>*> m_inputHandles;
  std::vector<DataObjectHandle<DataObject>*> m_outputHandles;

  static CHM m_name_ncopies_map;
};
