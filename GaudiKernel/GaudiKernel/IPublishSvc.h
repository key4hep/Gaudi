#ifndef GAUDIKERNEL_IPUBLISH_H
#define GAUDIKERNEL_IPUBLISH_H

#include "GaudiKernel/IInterface.h"
#include <string>


/** @class IPublishSvc IPublishSvc.h GaudiKernel/IPublishSvc.h

    Definition of the IPublishSvc interface, which publishes Gaudi variables
    to outside monitoring processes.

    @author Beat Jost
*/
class GAUDI_API IPublishSvc: virtual public IInterface
{
public:
  /// InterfaceID
  DeclareInterfaceID(IPublishSvc, 1, 0);

  /** Declare publishing information
    @param name Information name knwon to the external system
    @param var address of the published entity
   */
  virtual void declarePubItem(const std::string& name, const bool&  var) = 0;
  virtual void declarePubItem(const std::string& name, const int&  var) = 0;
  virtual void declarePubItem(const std::string& name, const long&  var) = 0;
  virtual void declarePubItem(const std::string& name, const double& var) = 0;
  virtual void declarePubItem(const std::string& name, const std::string& var) = 0;

  /** Undeclare publishing information
    @param name publishing information name known to the external system
   */

  virtual void undeclarePubItem( const std::string& name ) = 0;
  /** Undeclare all publishing information
   */

  virtual void undeclarePubAll() = 0;
  /** Update pblishing information
      @param name publishing information name known to the external system
   */

  virtual void updateItem(const std::string &name) = 0;

  /** Update all declared publishing items
   */
  virtual void updateAll() = 0;

  /// Virtual destructor.
  virtual ~IPublishSvc() {}
};

#endif // GAUDIKERNEL_IPUBLISH_H
