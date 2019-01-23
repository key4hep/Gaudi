// Dear emacs, this is -*- c++ -*-
#ifndef GAUDIKERNEL_SYSTEMLINUX_H
#define GAUDIKERNEL_SYSTEMLINUX_H

// System include(s):
#include <string>
#include <vector>

namespace System
{
  /// Namespace holding Linux specific functions
  namespace Linux
  {

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

  } // namespace Linux

  /// The platform name to use in the "main" functions
  namespace Platform = Linux;

} // namespace System

#endif // GAUDIKERNEL_SYSTEMLINUX_H
