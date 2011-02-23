// $Id: UpdateManagerException.h,v 1.1 2005/12/20 14:06:19 hmd Exp $
#ifndef GAUDIKERNEL_UPDATEMANAGEREXCEPTION_H
#define GAUDIKERNEL_UPDATEMANAGEREXCEPTION_H 1

// Include files
#include "GaudiKernel/GaudiException.h"

/** @class UpdateManagerException UpdateManagerException.h GaudiKernel/UpdateManagerException.h
 *
 *  Exception thrown by the UpdateManagerSvc when something goes wrong.
 *
 *  @see GaudiException
 *
 *  @author Marco Clemencic
 *  @date   2005-12-14
 */
class GAUDI_API UpdateManagerException: public GaudiException {
public:
  /// Standard constructor
  UpdateManagerException( const std::string& Message = "unspecified exception",
                          const std::string& Tag = "*UpdateManagerSvc*",
                          const StatusCode & Code = StatusCode::FAILURE ):
    GaudiException(Message,Tag,Code) {}

  UpdateManagerException( const std::string& Message,
                          const std::string& Tag,
                          const StatusCode & Code,
                          const GaudiException& Ex ):
    GaudiException(Message,Tag,Code,Ex) {}

  /// Destructor.
  virtual ~UpdateManagerException() throw () {}

  /// Clone the exception.
  virtual UpdateManagerException* clone() const { return new UpdateManagerException(*this); }

};
#endif // GAUDIKERNEL_UPDATEMANAGEREXCEPTION_H
