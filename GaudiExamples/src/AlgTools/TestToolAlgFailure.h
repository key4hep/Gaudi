#ifndef TESTTOOLALGFAILURE_H
#define TESTTOOLALGFAILURE_H 1

// from STL
#include <string>

// base class
#include "GaudiAlg/GaudiAlgorithm.h"

/** @class TestToolAlgFailure
 *
 *  Retrieve tools ignoring errors.
 *
 *  @author Marco Clemencic
 *  @date   2008-10-22
 */

class TestToolAlgFailure : public GaudiAlgorithm
{

public:
  /// Standard constructor
  using GaudiAlgorithm::GaudiAlgorithm;

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  Gaudi::Property<std::vector<std::string>> m_tools{this, "Tools", {}, "list of tools to test"};
  Gaudi::Property<bool>                     m_ignoreFailure{this, "IgnoreFailure", false};
};

#endif // TESTTOOLALG_H
