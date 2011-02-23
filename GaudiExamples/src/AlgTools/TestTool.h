// $Id: TestTool.h,v 1.2 2004/07/12 13:30:19 mato Exp $
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

class TestTool : public extends1<GaudiTool, ITestTool> {

public:

  /// Standard constructor
  TestTool( const std::string& type,
            const std::string& name,
            const IInterface* parent);

  /// Initialize method
  StatusCode initialize();

  /// Finalize method
  StatusCode finalize();

  virtual ~TestTool( ) {} ///< Destructor

private:

  // list of tools to test
  typedef std::vector<std::string> ToolList;
  ToolList m_tools;

};
#endif // TESTTOOL_H
