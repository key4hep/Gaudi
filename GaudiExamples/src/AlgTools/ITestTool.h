// $Id: ITestTool.h,v 1.1 2004/03/09 09:25:32 mato Exp $
#ifndef ITESTTOOL_H
#define ITESTTOOL_H 1

// Include files
// from STL
#include <string>

// from Gaudi
#include "GaudiKernel/IAlgTool.h"


/** @class ITestTool ITestTool.h
 *
 *
 *  @author Chris Jones
 *  @date   2004-03-08
 */

class ITestTool : public virtual IAlgTool {

public:
  /// InterfaceID
  DeclareInterfaceID(ITestTool,2,0);
  virtual ~ITestTool() {}
protected:

private:

};
#endif // ITESTTOOL_H
