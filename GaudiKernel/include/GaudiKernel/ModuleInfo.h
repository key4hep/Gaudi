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
#include <string>
#include <vector>

/** ModuleInfo: OS specific details to access at run-time the module
    configuration of the process.

    M.Frank
*/
namespace System {
  enum ModuleType { UNKNOWN, SHAREDLIB, EXECUTABLE };
  /// Definition of an image handle
  typedef void* ImageHandle;
  /// Definition of the process handle
  typedef void* ProcessHandle;
  /// Get the name of the (executable/DLL) file without file-type
  GAUDI_API const std::string& moduleName();
  /// Get the full name of the (executable/DLL) file
  GAUDI_API const std::string& moduleNameFull();
  /// Get type of the module
  GAUDI_API ModuleType moduleType();
  /// Handle to running process
  GAUDI_API ProcessHandle processHandle();
  /// Handle to currently executed module
  GAUDI_API ImageHandle moduleHandle();
  /// Handle to the executable file running
  GAUDI_API ImageHandle exeHandle();
  /// Name of the executable file running
  GAUDI_API const std::string& exeName();
  /// Vector of names of linked modules
  GAUDI_API const std::vector<std::string> linkedModules();
  /// Attach module handle
  GAUDI_API void setModuleHandle( ImageHandle handle );
} // namespace System
