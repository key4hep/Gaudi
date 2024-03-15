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
#ifndef TESTTOOLALGFAILURE_H
#define TESTTOOLALGFAILURE_H 1

// from STL
#include <string>

// base class
#include "GaudiKernel/Algorithm.h"

/** @class TestToolAlgFailure
 *
 *  Retrieve tools ignoring errors.
 *
 *  @author Marco Clemencic
 *  @date   2008-10-22
 */

class TestToolAlgFailure : public Algorithm {

public:
  /// Standard constructor
  using Algorithm::Algorithm;

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  Gaudi::Property<std::vector<std::string>> m_tools{ this, "Tools", {}, "list of tools to test" };
  Gaudi::Property<bool>                     m_ignoreFailure{ this, "IgnoreFailure", false };
  Gaudi::Property<bool>                     m_throwException{ this, "ThrowException", false };
};

#endif // TESTTOOLALG_H
