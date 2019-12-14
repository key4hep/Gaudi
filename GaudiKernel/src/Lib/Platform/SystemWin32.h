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
// Dear emacs, this is -*- c++ -*-
#ifndef GAUDIKERNEL_SYSTEMWIN32_H
#define GAUDIKERNEL_SYSTEMWIN32_H

// System include(s):
#include <string>
#include <vector>

namespace System {
  /// Namespace holding Windows specific functions
  namespace Win32 {

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

  } // namespace Win32

  /// The platform name to use in the "main" functions
  namespace Platform = Win32;

} // namespace System

#endif // GAUDIKERNEL_SYSTEMWIN32_H
