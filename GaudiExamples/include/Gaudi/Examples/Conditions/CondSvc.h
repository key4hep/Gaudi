/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include "Gaudi/Property.h"
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/ICondSvc.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SmartIF.h"

namespace Gaudi::Examples::Conditions {

  /// Implementation of ICondSvc used for testing
  /// Allows declaration of one or more algorithms or data objects
  /// as belonging to the "conditions realm"
  /// The scheduler will then treat them differently
  class CondSvc : public extends<Service, ICondSvc> {

  public:
    using extends::extends;

    StatusCode initialize() override;

    StatusCode regHandle( IAlgorithm*, const Gaudi::DataHandle& ) override { return StatusCode::SUCCESS; };

    /// check to see if a specific condition object ID is valid for this event
    bool isValidID( const EventContext&, const DataObjID& ) const override { return false; };

    /// get list of all registered condition Algorithms
    const std::set<IAlgorithm*>& condAlgs() const override { return m_condAlgs; };

    /// query if a specific Algorithm is a registered condition Algorithm
    bool isRegistered( IAlgorithm* alg ) const override { return ( m_condAlgs.find( alg ) != m_condAlgs.end() ); };

    /// query if a condition Object ID is registered
    bool isRegistered( const DataObjID& id ) const override { return ( m_condData.find( id ) != m_condData.end() ); };

    /// get collection of all registered condition Object IDs
    const DataObjIDColl& conditionIDs() const override { return m_condData; }

    /// dump the condition store
    void dump( std::ostream& ) const override{};

    /// retrieve all valid ranges for one Object ID
    StatusCode validRanges( std::vector<EventIDRange>&, const DataObjID& ) const override {
      return StatusCode::SUCCESS;
    };

    /// Asynchronously setup conditions
    ConditionSlotFuture* startConditionSetup( const EventContext& ) override { return nullptr; };

    /// register an IConditionIOSvc (alternative to Algorithm processing of
    /// Conditions)
    StatusCode registerConditionIOSvc( IConditionIOSvc* ) override { return StatusCode::SUCCESS; }

  private:
    std::set<IAlgorithm*>                     m_condAlgs;
    Gaudi::Property<std::vector<std::string>> m_algNames{this, "Algs", {}, "Names of conditions algorithms"};

    DataObjIDColl                             m_condData;
    Gaudi::Property<std::vector<std::string>> m_dataNames{this, "Data", {}, "Names of conditions data"};

    SmartIF<IAlgResourcePool> m_algResourcePool;
  };

  DECLARE_COMPONENT( CondSvc )
} // namespace Gaudi::Examples::Conditions
