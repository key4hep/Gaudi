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

// Framework include files
// #include "GaudiKernel/DataObject.h"

// Forward declarations
class DataObject;

// Functions imported from GaudiKernel
namespace GaudiRoot {
  void popCurrentDataObject();
  void pushCurrentDataObject( DataObject** pobjAddr );
} // namespace GaudiRoot

namespace {

  /** @struct  DataObjectPush RootUtils.h src/RootUtils.h
   *
   * Helper structure to push data object onto execution stack.
   * This way the execution context for SmartRef objects is set
   * and Gaudi references can easily be initialized.
   *
   * @author  M.Frank
   * @version 1.0
   */
  struct DataObjectPush {
    /// Reference to DataObject
    DataObject* m_p;
    /// Initializing constructor
    DataObjectPush( DataObject* p ) : m_p( p ) { GaudiRoot::pushCurrentDataObject( &m_p ); }
    /// Stnadard destructor. Note: NOT virtual, hence no inheritance!
    ~DataObjectPush() { GaudiRoot::popCurrentDataObject(); }
  };
} // namespace
