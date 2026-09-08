/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <Gaudi/AsynchronousAlgorithm.h>
#include <GaudiKernel/DataObjectHandle.h>
#include <GaudiKernel/IRndmGenSvc.h>
#include <GaudiKernel/RegistryEntry.h>
#include <GaudiKernel/RndmGenerators.h>
#include <tbb/concurrent_hash_map.h>

//------------------------------------------------------------------------------

/** @class GPUCruncher
 *
 *  A test asynchronous algorithm on the GPU
 */
class GPUCruncher : public Gaudi::AsynchronousAlgorithm {

public:
  bool isClonable() const override { return true; }

  /// the execution of the algorithm
  StatusCode execute( const EventContext& ctx ) const override;
  /// Its initialization
  StatusCode initialize() override;
  /// the finalization of the algorithm
  StatusCode finalize() override;

  GPUCruncher( const std::string& name, // the algorithm instance name
               ISvcLocator*       pSvc );     // the Service Locator

  /// virtual & protected destructor
  virtual ~GPUCruncher(); // virtual & protected destructor

private:
  /// the default constructor is disabled
  GPUCruncher(); // no default constructor
  /// the copy constructor is disabled
  GPUCruncher( const GPUCruncher& ); // no copy constructor
  /// the assignment operator is disabled
  GPUCruncher& operator=( const GPUCruncher& ); // no assignment
  /// The GPU intensive function
  StatusCode gpuExecute( const std::vector<double>& in, std::vector<double>& out ) const;

  Gaudi::Property<std::vector<std::string>> m_inpKeys{ this, "inpKeys", {}, "" };
  Gaudi::Property<std::vector<std::string>> m_outKeys{ this, "outKeys", {}, "" };

  Gaudi::Property<double> m_avg_runtime{ this, "avgRuntime", 1., "Average runtime of the module." };
  Gaudi::Property<double> m_var_runtime{ this, "varRuntime", 0.01, "Variance of the runtime of the module." };

  std::vector<DataObjectHandle<DataObject>*> m_inputHandles;
  std::vector<DataObjectHandle<DataObject>*> m_outputHandles;

  using CHM = tbb::concurrent_hash_map<std::string, unsigned int>;
  static CHM m_name_ncopies_map;
};
