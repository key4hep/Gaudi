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
//	SystemBase.h
//--------------------------------------------------------------------
//
//	Package    : System (The LHCb System service)
//
//  Description: Definition of Systems internals
//
//	Author     : M.Frank
//  Created    : 13/11/00
//	Changes    :
//====================================================================
#ifndef GAUDI_SYSTEM_SYSTEMBASE_H
#define GAUDI_SYSTEM_SYSTEMBASE_H

namespace System {
  /// Enumeration for fetching information
  enum InfoType { NoFetch, RemainTime, Times, ProcessBasics, PriorityBoost, Memory, Quota, System, Modules, IO };
} // namespace System
#endif // GAUDI_SYSTEM_SYSTEMBASE_H
