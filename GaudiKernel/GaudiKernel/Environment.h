//====================================================================
//	Timing.h
//--------------------------------------------------------------------
//
//	Package    : Gaudi/System (The LHCb System service)
//
//  Description: Definition of Systems internals
//
//	Author     : M.Frank
//  Created    : 13/1/99
//	Changes    :
//====================================================================
#ifndef GAUDIKERNEL_ENVIRONMENT_H
#define GAUDIKERNEL_ENVIRONMENT_H

// Framework include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/SystemBase.h"

// C++ include files
#include <string>

/** Note: OS specific details for environment resolution

    Entrypoints:

    @author:  M.Frank
    @version: 1.0
*/
namespace System
{
  GAUDI_API std::string homeDirectory();
  GAUDI_API std::string tempDirectory();
  GAUDI_API StatusCode resolveEnv( const std::string& var, std::string& res, int recusions = 124 );
}
#endif // GAUDIKERNEL_ENVIRONMENT_H
