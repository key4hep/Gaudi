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

class TestToolAlgFailure : public GaudiAlgorithm {

public:

  /// Standard constructor
  TestToolAlgFailure( const std::string& name, ISvcLocator* pSvcLocator );

  ~TestToolAlgFailure( ) override; ///< Destructor

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:

  // list of tools to test
  typedef std::vector<std::string> ToolList;
  ToolList m_tools;

  bool m_ignoreFailure;

};

#endif // TESTTOOLALG_H
