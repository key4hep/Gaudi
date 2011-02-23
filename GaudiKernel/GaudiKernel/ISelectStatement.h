// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/ISelectStatement.h,v 1.3 2003/01/27 13:48:41 mato Exp $
#ifndef GAUDIKERNEL_ISELECTSTATEMENT_H
#define GAUDIKERNEL_ISELECTSTATEMENT_H

// STL include files
#include <string>

// Framework include files
#include "GaudiKernel/IInterface.h"

/** @class ISelectStatement ISelectStatement.h GaudiKernel/ISelectStatement.h

  A select statement can either contain
  - a string e.g. for refining an SQL statement
  - a function object, which will be called back
    in order to refine a selection.
    This happens in calling sequences like the following:

  bool MySelect::operator()(IValueLocator* l)   {
    float px, py, pz;
    if ( l->get("PX",px) && l->get("PY",py) && l->get("PZ",pz) )  {
      float mom = sqrt(px*px+py*py+pz*pz);
      return mom > 100.0 * GeV;
    }
    return false;
  }

    if "true" is returned, the object will be loaded completely.

    @author  M.Frank
    @version 1.0
*/
class GAUDI_API ISelectStatement: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(ISelectStatement,2,0);

  /// Statement type definition
  enum SelectType { FUNCTION=1<<1, STRING=1<<2, FULL=1<<3, OTHER=1<<4 };

public:
  /// Access the type of the object
  virtual long type()   const  = 0;
  /// Access the selection string
  virtual const std::string& criteria()  const = 0;
  /// Set the type
  virtual void setCriteria(const std::string& crit) = 0;
  /// Change activity flag
  virtual void setActive(bool flag = true) = 0;
  /// Check if selection is active
  virtual bool isActive()   const = 0;
  /// Stupid default implementation
  virtual bool operator()(void* val) = 0;
};


#endif  // GAUDIKERNEL_ISELECTSTATEMENT_H
