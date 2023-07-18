/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include "ITestTool.h"
#include <GaudiKernel/AlgTool.h>
#include <string>

/** @class TestTool TestTool.h
 *
 *
 *  @author Chris Jones
 *  @date   2004-03-08
 */

class TestTool : public extends<AlgTool, ITestTool> {

public:
  /// Standard constructor
  using extends::extends;

  /// Initialize method
  StatusCode initialize() override;

  /// Finalize method
  StatusCode finalize() override;

private:
  Gaudi::Property<std::vector<std::string>> m_tools{ this, "Tools", {}, "list of tools to test" };
};
