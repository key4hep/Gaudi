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

// Include files
#include <GaudiKernel/IService.h>
#include <string>

// forward declarations
class StatEntity;
class Stat;

namespace Gaudi {
  namespace CounterSummary {
    /// How is the counter to be saved?
    enum SaveType {
      SaveSimpleCounter = 0,   /// store only the flag()
      SaveStatEntity,          /// store all information
      SaveAlwaysSimpleCounter, /// store always, store only the flag()
      SaveAlwaysStatEntity     /// store always, store all information
    };
  } // namespace CounterSummary
} // namespace Gaudi

/** @class ICounterSummarySvc ICounterSummarySvc.h
 *
 *  Simple service interface to collect counters to persist in a summary file.
 *  In LHCb this is used to write an XML summary of the job.
 *
 *  @see XMLSummarySvc
 *
 *  @author Rob Lambert
 *  @date   2009-07-29
 */
class GAUDI_API ICounterSummarySvc : virtual public IService {

public:
  /// InterfaceID
  DeclareInterfaceID( ICounterSummarySvc, 1, 0 );

  /** declare a counter, StatEntity, to be filled in the Counter summary
   *  @param  std::string  [IN]:  Name of the tool/alg/mother filling this counter
   *  @param  std::string name  [IN]:  Name of the counter
   *  @param  StatEntity   [IN]: The counter to store/save
   *  @param  saveType   [IN]: How to save it, from an enum
   *
   *  @see Gaudi::CounterSummary::SaveType
   */
  virtual void
  addCounter( const std::string, const std::string name, const StatEntity&,
              const Gaudi::CounterSummary::SaveType saveType = Gaudi::CounterSummary::SaveSimpleCounter ) = 0;

  /** declare a counter, Stat, to be filled in the Counter summary
   *  @param  std::string  [IN]:  Name of the tool/alg/mother filling this counter
   *  @param  std::string name  [IN]:  Name of the counter
   *  @param  Stat   [IN]: The counter to store/save
   *  @param  saveType   [IN]: How to save it, from an enum
   *
   *  @see Gaudi::CounterSummary::SaveType
   */
  virtual void
  addCounter( const std::string, const std::string name, const Stat&,
              const Gaudi::CounterSummary::SaveType saveType = Gaudi::CounterSummary::SaveSimpleCounter ) = 0;
};
