/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/IAuditor.h>

namespace Gaudi {

  const std::string IAuditor::Initialize   = "Initialize";
  const std::string IAuditor::ReInitialize = "ReInitialize";
  const std::string IAuditor::Start        = "Start";
  const std::string IAuditor::ReStart      = "ReStart";
  const std::string IAuditor::Execute      = "Execute";
  const std::string IAuditor::Stop         = "Stop";
  const std::string IAuditor::Finalize     = "Finalize";

} // namespace Gaudi
