#ifndef GAUDIKERNEL_IDATASTORELEAVES_H
#define GAUDIKERNEL_IDATASTORELEAVES_H

// Include Files
#include "GaudiKernel/IAlgTool.h"
#include <vector>

class DataObject;

/** Interface for a tool or service that returns a list of leaves (paths) in a
 *  transient store.
 *
 *  @author Marco Clemencic
 */
class GAUDI_API IDataStoreLeaves: virtual public IAlgTool {

public:
  /// InterfaceID
  DeclareInterfaceID(IDataStoreLeaves,1,0);

  /// Returned type
  typedef std::vector<DataObject*> LeavesList;

  /// Return the list of leaves.
  virtual const LeavesList & leaves() const = 0;

};

#endif
