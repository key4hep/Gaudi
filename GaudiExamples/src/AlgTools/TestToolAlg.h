// $Id: TestToolAlg.h,v 1.1 2004/03/09 09:25:33 mato Exp $
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

  virtual ~TestToolAlg( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

private:

  // list of tools to test
  typedef std::vector<std::string> ToolList;
  ToolList m_tools;

};

#endif // TESTTOOLALG_H
