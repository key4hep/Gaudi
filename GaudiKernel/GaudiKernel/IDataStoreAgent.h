#ifndef GAUDIKERNEL_IDATASTOREAGENT_H
#define GAUDIKERNEL_IDATASTOREAGENT_H

// Framework include files
#include "GaudiKernel/Kernel.h"

// Forward declarations:
#include "GaudiKernel/IRegistry.h"

/** @class IDataStoreAgent IDataStoreAgent.h GaudiKernel/IDataStoreAgent.h

    Generic data agent interface

    @author Markus Frank
*/
class GAUDI_API IDataStoreAgent
{
public:
  /// destructor
  virtual ~IDataStoreAgent() = default;

  /** Analyse the data object.
  @return Boolean indicating wether the tree below should be analysed
  */
  virtual bool analyse( IRegistry* pObject, int level ) = 0;
};
#endif // GAUDIKERNEL_IDATASTOREAGENT_H
