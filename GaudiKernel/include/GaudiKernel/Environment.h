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

#include <GaudiKernel/Kernel.h>
#include <GaudiKernel/StatusCode.h>
#include <GaudiKernel/SystemBase.h>
#include <string>

/** Note: OS specific details for environment resolution

    Entrypoints:

    @author:  M.Frank
    @version: 1.0
*/
namespace System {
  GAUDI_API std::string homeDirectory();
  GAUDI_API std::string tempDirectory();
  GAUDI_API StatusCode  resolveEnv( const std::string& var, std::string& res, int recusions = 124 );
} // namespace System
