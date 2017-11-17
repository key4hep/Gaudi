#ifndef TESTTOOL_H
#define TESTTOOL_H 1

// Include files
// from STL
#include <string>

// from Gaudi
#include "GaudiAlg/GaudiTool.h"

#include "ITestTool.h"

/** @class TestTool TestTool.h
 *
 *
 *  @author Chris Jones
 *  @date   2004-03-08
 */

class TestTool : public extends<GaudiTool, ITestTool>
{

public:
  /// Standard constructor
  using extends::extends;

  /// Initialize method
  StatusCode initialize() override;

  /// Finalize method
  StatusCode finalize() override;

private:
  Gaudi::Property<std::vector<std::string>> m_tools{this, "Tools", {}, "list of tools to test"};
};
#endif // TESTTOOL_H
