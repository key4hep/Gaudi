// $Id: IMyAudTool.h,v 1.1 2007/01/22 16:06:11 hmd Exp $
#ifndef GAUDIEXAMPLE_IMYAUDTOOL_H
#define GAUDIEXAMPLE_IMYAUDTOOL_H 1

// Include files
#include "GaudiKernel/IAlgTool.h"

/** @class IMyAudTool IMyAudTool.h
 *  Example of an Interface of a Algorithm Tool
 *
 *  @author Pere Mato
*/
class IMyAudTool : virtual public IAlgTool {
public:
  /// InterfaceID
  DeclareInterfaceID(IMyAudTool,2,0);

  /// Get a message
  virtual const std::string&  message() const = 0;
  /// Do something
  virtual void  doErr() = 0;
  virtual void  doFatal() = 0;
};

#endif // GAUDIEXAMPLE_IMYAUDTOOL_H
