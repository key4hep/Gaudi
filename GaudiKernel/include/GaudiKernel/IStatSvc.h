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

#include <GaudiKernel/IInterface.h>
#include <string>

class StatEntity;

/** @class IStatSvc IStatSvc.h GaudiKernel/IStatSvc.h
 *  "Stat"-related part of interface IChronoStatSvc
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date December 1, 1999
 *  @date   2007-07-08
 */
class GAUDI_API IStatSvc : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IStatSvc, 2, 0 );
  /// the actual identificator of Stat
  typedef std::string StatTag;
  /// the actual value type used for Stat
  typedef double StatFlag;

public:
  /** add statistical information to the entity , tagged by its name
   *   @param t stat tag(name)
   *   @param f flag (quantity to be accumulated)
   *   @return status of stat
   */
  virtual void stat( const StatTag& t, const StatFlag& f ) = 0;
  /** prints (using message service)  info about statistical entity, tagged by its name
   *  @param t stat tag(name)
   *  @return status code
   */
  virtual void statPrint( const StatTag& t ) = 0;
  /** extract the stat   entity for the given tag (name)
   *  @param t stat   tag(name)
   *  @return pointer to stat   entity
   */
  virtual StatEntity* stat( const StatTag& t ) = 0;
};
