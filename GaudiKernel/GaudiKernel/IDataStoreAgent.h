// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IDataStoreAgent.h,v 1.4 2006/11/30 15:22:07 dquarrie Exp $
#ifndef GAUDIKERNEL_IDATASTOREAGENT_H
#define GAUDIKERNEL_IDATASTOREAGENT_H

// Framework include files
#include "GaudiKernel/Kernel.h"

// Forward declarations:
class IRegistry;

/** @class IDataStoreAgent IDataStoreAgent.h GaudiKernel/IDataStoreAgent.h

    Generic data agent interface

    @author Markus Frank
*/
class GAUDI_API IDataStoreAgent {
public:
  /// destructor
  virtual ~IDataStoreAgent() { }

  /** Analyse the data object.
  @return Boolean indicating wether the tree below should be analysed
  */
  virtual bool analyse(IRegistry* pObject, int level) = 0;
};
#endif // GAUDIKERNEL_IDATASTOREAGENT_H
