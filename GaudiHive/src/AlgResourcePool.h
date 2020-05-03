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
#ifndef GAUDIHIVE_ALGRESOURCEPOOL_H
#define GAUDIHIVE_ALGRESOURCEPOOL_H

#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Service.h"
#include <Gaudi/Algorithm.h>

#include <atomic>
#include <bitset>
#include <list>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

// External libs
#include "boost/dynamic_bitset.hpp"
#include "tbb/concurrent_queue.h"

/** @class AlgResourcePool AlgResourcePool.h GaudiHive/AlgResourcePool.h

    The AlgResourcePool is a concrete implementation of the IAlgResourcePool interface.
    It either creates all instances up front or lazily.
    Internal bookkeeping is done via hashes of the algorithm names.

    @author Benedikt Hegner
*/
class AlgResourcePool : public extends<Service, IAlgResourcePool> {
public:
  // Standard constructor
  using extends::extends;
  // Standard destructor
  ~AlgResourcePool() override;

  StatusCode start() override;
  StatusCode initialize() override;
  /// Acquire a certain algorithm using its name
  StatusCode acquireAlgorithm( std::string_view name, IAlgorithm*& algo, bool blocking = false ) override;
  /// Release a certain algorithm
  StatusCode releaseAlgorithm( std::string_view name, IAlgorithm*& algo ) override;
  /// Acquire a certain resource
  StatusCode acquireResource( std::string_view name ) override;
  /// Release a certain resource
  StatusCode releaseResource( std::string_view name ) override;

  std::list<IAlgorithm*> getFlatAlgList() override;
  std::list<IAlgorithm*> getTopAlgList() override;

  StatusCode stop() override;

private:
  typedef tbb::concurrent_bounded_queue<IAlgorithm*> concurrentQueueIAlgPtr;
  typedef std::list<SmartIF<IAlgorithm>>             ListAlg;
  typedef boost::dynamic_bitset<>                    state_type;

  std::mutex m_resource_mutex;

  state_type                                m_available_resources{0};
  std::map<size_t, concurrentQueueIAlgPtr*> m_algqueue_map;
  std::map<size_t, state_type>              m_resource_requirements;
  std::map<size_t, size_t>                  m_n_of_allowed_instances;
  std::map<size_t, unsigned int>            m_n_of_created_instances;
  std::map<std::string_view, unsigned int>  m_resource_indices;

  /// Decode the top Algorithm list
  StatusCode decodeTopAlgs();

  /// Recursively flatten an algList
  StatusCode flattenSequencer( Gaudi::Algorithm* sequencer, ListAlg& alglist, unsigned int recursionDepth = 0 );

  Gaudi::Property<bool>                     m_lazyCreation{this, "CreateLazily", false, ""};
  Gaudi::Property<std::vector<std::string>> m_topAlgNames{
      this, "TopAlg", {}, "Names of the algorithms to be passed to the algorithm manager"};
  Gaudi::Property<bool> m_overrideUnClonable{this, "OverrideUnClonable", false,
                                             "Override the un-clonability of algorithms. Use with caution!"};

  /// The list of all algorithms created within the Pool which are not top
  ListAlg m_algList;

  /// The list of top algorithms
  ListAlg m_topAlgList;

  /// The flat list of algorithms w/o clones
  ListAlg m_flatUniqueAlgList;

  /// The flat list of algorithms w/o clones which is returned
  std::list<IAlgorithm*> m_flatUniqueAlgPtrList;

  /// The top list of algorithms
  std::list<IAlgorithm*> m_topAlgPtrList;
};

#endif // GAUDIHIVE_ALGRESOURCEPOOL_H
