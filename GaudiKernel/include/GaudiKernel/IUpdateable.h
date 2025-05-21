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
#include <GaudiKernel/StatusCode.h>

/** @class IUpdateable IUpdateable.h GaudiKernel/IUpdateable.h
 *
 *  Object update interface definition.
 *  Definition of a interface to recalculate object properties.
 *
 *   @author  M.Frank
 *   @version 1.0
 */
class GAUDI_API IUpdateable {
public:
  /// Default destructor
  virtual ~IUpdateable() = default;

  /** Output serialization to stream buffer. Saves the status of the object.
   * @param   flag    Flag to trigger implementation specific action.
   *
   * @return  Status code indicating success or failure.
   */
  virtual StatusCode update( int flag ) = 0;
};

/** @class IUpdateableInterface IUpdateable.h GaudiKernel/IUpdateable.h
 *
 *  Component interface counterpart to the above ABC.
 *
 *   @author  M.Frank
 *   @version 1.0
 */
class GAUDI_API IUpdateableIF : virtual public IInterface, virtual public IUpdateable {
public:
  /// InterfaceID
  DeclareInterfaceID( IUpdateableIF, 1, 0 );
};
