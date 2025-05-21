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
// from STL
#include <string>

// from Gaudi
#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/Time.h>

/** @class IEventTimeDecoder IEventTimeDecoder.h GaudiKernel/IEventTimeDecoder.h
 *
 *  Interface that a Tool that decodes the event time has to implement.
 *
 *  @author Marco Clemencic
 *  @date   2006-09-21
 */
class GAUDI_API IEventTimeDecoder : virtual public IAlgTool {
public:
  /// InterfaceID
  DeclareInterfaceID( IEventTimeDecoder, 2, 0 );

  /// Return the time of current event.
  virtual Gaudi::Time getTime() const = 0;
};
