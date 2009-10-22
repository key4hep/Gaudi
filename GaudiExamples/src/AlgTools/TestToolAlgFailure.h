// $Id: TestToolAlgFailure.h,v 1.1 2008/10/22 14:49:16 marcocle Exp $
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

  virtual ~TestToolAlgFailure( ); ///< Destructor

  virtual StatusCode initialize();
  virtual StatusCode execute();
  virtual StatusCode finalize();

private:

  // list of tools to test
  typedef std::vector<std::string> ToolList;
  ToolList m_tools;

  bool m_ignoreFailure;

};

#endif // TESTTOOLALG_H
