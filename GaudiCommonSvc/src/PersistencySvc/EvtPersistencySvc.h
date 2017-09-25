//	============================================================
//
//	PersistencySvc.h
//	------------------------------------------------------------
//
//	Package   : PersistencySvc
//
//	Author    : Markus Frank
//
//	===========================================================
#ifndef PERSISTENCYSVC_EVTPERSISTENCYSVC_H
#define PERSISTENCYSVC_EVTPERSISTENCYSVC_H 1

#include "PersistencySvc.h"

/** EvtPersistencySvc class implementation definition.

    <P> System:  The LHCb Offline System
    <P> Package: EvtPersistencySvc

     Dependencies:
    <UL>
    <LI> PersistencySvc definition:  "Kernel/Interfaces/PersistencySvc.h"
    </UL>

    History:
    <PRE>
    +---------+----------------------------------------------+---------+
    |    Date |                 Comment                      | Who     |
    +---------+----------------------------------------------+---------+
    | 3/11/98 | Initial version                              | M.Frank |
    +---------+----------------------------------------------+---------+
    </PRE>
   @author Markus Frank
   @version 1.0
*/
class EvtPersistencySvc : virtual public PersistencySvc
{
public:
  /**@name: Object implementation  */
  //@{
  /// Standard Constructor
  EvtPersistencySvc( const std::string& name, ISvcLocator* svc );

  /// Standard Destructor
  ~EvtPersistencySvc() override = default;
};
#endif // PERSISTENCYSVC_EVTPERSISTENCYSVC_H
