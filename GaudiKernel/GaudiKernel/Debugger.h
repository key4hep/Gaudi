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
//====================================================================
//	Debugger.h
//--------------------------------------------------------------------
//
//	Package    : System (The LHCb System service)
//
//  Description: Invoke interactively the debugger from a
//               running application
//
//	Author     : M.Frank
//  Created    : 13/1/99
//	Changes    :
//====================================================================
#ifndef GAUDI_SYSTEM_DEBUGGER_H
#define GAUDI_SYSTEM_DEBUGGER_H

#include "GaudiKernel/Kernel.h"

namespace System {
  /// Break the execution of the application and invoke the debugger
  GAUDI_API long breakExecution();
  /// Break the execution of the application and invoke the debugger in a remote process
  GAUDI_API long breakExecution( long pid );
} // namespace System
#endif // GAUDI_SYSTEM_DEBUGGER_H
