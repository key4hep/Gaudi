/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiKernel/Service.h"

#include "MyTool.h"

namespace GaudiTesting {

  /** Special service that issue a failure in one of the transitions (for testing).
   */
  class SvcWithTool : public Service {
  public:
    using Service::Service;

  private:
    PublicToolHandle<IMyTool> m_tool{this, "MyTool", "MyTool"};
    ToolHandle<IMyTool> m_privTool{this, "MyPrivateTool", ""};
  };

  DECLARE_COMPONENT( SvcWithTool )
} // namespace GaudiTesting
