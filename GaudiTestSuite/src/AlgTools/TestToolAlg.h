/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef TESTTOOLALG_H
#define TESTTOOLALG_H 1

// from STL
#include <string>

// base class
#include <GaudiKernel/Algorithm.h>

/** @class RichToolTest RichToolTest.h component/RichToolTest.h
 *
 *
 *  @author Chris Jones
 *  @date   2004-03-08
 */

class TestToolAlg : public Algorithm {

public:
  /// Standard constructor
  using Algorithm::Algorithm;

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization

private:
  Gaudi::Property<std::vector<std::string>> m_tools{ this, "Tools", {}, "list of tools to test" };
};

#endif // TESTTOOLALG_H
