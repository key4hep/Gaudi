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

class IAppMgrUI;
class ISvcLocator;
class IInterface;

/** @class Gaudi Bootstrap.h GaudiKernel/Bootstrap.h

    This is a number of static methods for bootstrapping the Gaudi framework

    @author Pere Mato
*/

namespace Gaudi {

  GAUDI_API IAppMgrUI*   createApplicationMgrEx( const std::string& dllname, const std::string& factname );
  GAUDI_API IAppMgrUI*   createApplicationMgr( const std::string& dllname, const std::string& factname );
  GAUDI_API IAppMgrUI*   createApplicationMgr( const std::string& dllname );
  GAUDI_API IAppMgrUI*   createApplicationMgr();
  GAUDI_API IInterface*  createInstance( const std::string& name, const std::string& factname,
                                         const std::string& ddlname );
  GAUDI_API ISvcLocator* svcLocator();
  /** Set new instance of service locator.
      @param    newInstance  Current instance of service locator.
      @return   Previous instance of service locator object.
  */
  GAUDI_API ISvcLocator* setInstance( ISvcLocator* newInstance );
  /** Set new instance of service locator.
      @param    newInstance  Current instance of service locator.
      @return   Previous instance of service locator object.
  */
  GAUDI_API IAppMgrUI* setInstance( IAppMgrUI* newInstance );
} // namespace Gaudi
