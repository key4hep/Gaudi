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

#include <GaudiKernel/IAlgTool.h>
#include <vector>

class DataObject;

/** Interface for a tool or service that returns a list of leaves (paths) in a
 *  transient store.
 *
 *  @author Marco Clemencic
 */
class GAUDI_API IDataStoreLeaves : virtual public IAlgTool {

public:
  /// InterfaceID
  DeclareInterfaceID( IDataStoreLeaves, 1, 0 );

  /// Returned type
  typedef std::vector<DataObject*> LeavesList;

  /// Return the list of leaves.
  virtual const LeavesList& leaves() const = 0;
};
