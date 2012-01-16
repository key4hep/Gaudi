#ifndef GAUDIKERNEL_IDODNODEMAPPER_H
#define GAUDIKERNEL_IDODNODEMAPPER_H 1
// Include files
// from STL
#include <string>

// from Gaudi
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/TypeNameString.h"

/** Interface of tools used by the DataOnDemandSvc to choose the type of node to
 * be created at a path.
 *
 * @author Marco Clemencic
 * @date 13/01/2012
 */
class IDODNodeMapper: virtual public IAlgTool {
public:
  /// InterfaceID
  DeclareInterfaceID(IDODNodeMapper, 1, 0);

  /// For the given path, returns a the type name of the object to be created at
  /// the path.
  virtual std::string nodeTypeForPath(const std::string &path) = 0;

protected:
private:
};

#endif // GAUDIKERNEL_IDODNODEMAPPER_H
