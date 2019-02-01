#ifndef TESTTOOLALG_H
#define TESTTOOLALG_H 1

// from STL
#include <string>

// base class
#include "GaudiAlg/GaudiAlgorithm.h"

/** @class RichToolTest RichToolTest.h component/RichToolTest.h
 *
 *
 *  @author Chris Jones
 *  @date   2004-03-08
 */

class TestToolAlg : public GaudiAlgorithm {

public:
  /// Standard constructor
  using GaudiAlgorithm::GaudiAlgorithm;

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization

private:
  Gaudi::Property<std::vector<std::string>> m_tools{this, "Tools", {}, "list of tools to test"};
};

#endif // TESTTOOLALG_H
