/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/DataHandle.h>
#include <GaudiKernel/DataObjID.h>
#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/EventIDRange.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/IService.h>

#include <set>
#include <sstream>
#include <string>

/**@class ICondSvc ICondSvc.h GaudiKernel/ICondSvc.h
 *
 *  Interface for the Condition Service. It allows Algorithms to register
 *  their ConditionsHanaldes with the Service, and clients to query the
 *  Service to determine which objects are valid/invalid for a particualar
 *  EventContext.
 *
 *  @author  Charles Leggett
 *  @version 2.0
 */

class ConditionSlotFuture;
class IConditionIOSvc;

class GAUDI_API ICondSvc : virtual public IService {
public:
  DeclareInterfaceID( ICondSvc, 1, 0 );

  /// register dependency between an Algorithm and the Condition Handle it creates
  virtual StatusCode regHandle( IAlgorithm* alg, const Gaudi::DataHandle& id ) = 0;

  /// check to see if a specific condition object ID is valid for this event
  virtual bool isValidID( const EventContext& ctx, const DataObjID& id ) const = 0;

  /// get list of all registered condition Algorithms
  virtual const std::set<IAlgorithm*>& condAlgs() const = 0;

  /// query if a specific Algorithm is a registered condition Algorithm
  virtual bool isRegistered( IAlgorithm* alg ) const = 0;

  /// query if a condition Object ID is registered
  virtual bool isRegistered( const DataObjID& id ) const = 0;

  /// get collection of all registered condition Object IDs
  virtual const DataObjIDColl& conditionIDs() const = 0;

  /// dump the condition store
  virtual void dump( std::ostream& ost ) const = 0;

  /// retrieve all valid ranges for one Object ID
  virtual StatusCode validRanges( std::vector<EventIDRange>& ranges, const DataObjID& id ) const = 0;

  /// Asynchronously setup conditions
  virtual ConditionSlotFuture* startConditionSetup( const EventContext& ctx ) = 0;

  /// register an IConditionIOSvc (alternative to Algorithm processing of
  /// Conditions)
  virtual StatusCode registerConditionIOSvc( IConditionIOSvc* ioService ) = 0;
};
