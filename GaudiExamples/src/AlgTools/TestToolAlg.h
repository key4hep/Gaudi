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
  TestToolAlg( const std::string& name, ISvcLocator* pSvcLocator );

  ~TestToolAlg() override; ///< Destructor

  StatusCode initialize() override;    ///< Algorithm initialization
  StatusCode execute   () override;    ///< Algorithm execution
  StatusCode finalize  () override;    ///< Algorithm finalization

private:

  // list of tools to test
  typedef std::vector<std::string> ToolList;
  ToolList m_tools;

};

#endif // TESTTOOLALG_H
