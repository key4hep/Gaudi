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
// Dear emacs, this is -*- c++ -*-
#pragma once

// System include(s):
#include <string>
#include <vector>

namespace System {
  /// Namespace holding MacOS specific functions
  namespace MacOS {

    /// Get the command line arguments of the process
    std::vector<std::string> cmdLineArgs();

    /// Get the human readable type name from a typeinfo name
    std::string typeinfoName( const char* name );

    /// Get the system's host name
    std::string hostName();

    /// Get the operating system's name
    std::string osName();

    /// Get the operating system's version
    std::string osVersion();

    /// Get the runner machine's type
    std::string machineType();

    /// Get the account name of the current user
    std::string accountName();

  } // namespace MacOS

  /// The platform name to use in the "main" functions
  namespace Platform = MacOS;

} // namespace System
