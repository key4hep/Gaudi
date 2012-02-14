#ifndef GAUDIKERNEL_IDODALGMAPPER_H
#define GAUDIKERNEL_IDODALGMAPPER_H 1
// Include files
// from STL
#include <string>

// from Gaudi
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/TypeNameString.h"

/** Interface of tools used by the DataOnDemandSvc to choose the algorithm to be
 * run to produce the data at a path.
 *
 * @author Marco Clemencic
 * @date 13/01/2012
 */
class IDODAlgMapper: virtual public IAlgTool {
public:
  /// InterfaceID
  DeclareInterfaceID(IDODAlgMapper, 1, 0);

  /// For the given path, returns a TypeNameString object identifying the algorithm
  /// to be run to produce the entry at the path.
  virtual Gaudi::Utils::TypeNameString algorithmForPath(const std::string &path) = 0;

protected:
private:
};

#endif // GAUDIKERNEL_IDODALGMAPPER_H
