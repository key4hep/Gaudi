#ifndef GAUDIALG_IGENERICTOOL_H
#define GAUDIALG_IGENERICTOOL_H 1

// Include files
#include "GaudiKernel/IAlgTool.h"

/** General purpose interface class for tools that "do something"
 *
 *  @author Marco Cattaneo
 *  @date   2005-12-14
 */
class GAUDI_API IGenericTool: virtual public IAlgTool {
public:
  /// InterfaceID
  DeclareInterfaceID(IGenericTool, 2, 0);

  /// Do the action
  virtual void execute() = 0;

};

/** General purpose interface class for tools that "do something"
 *  in a thread safe way.
 *
 *  @author Marco Clemencic
 *  @date   2016-12-09
 */
class GAUDI_API IGenericThreadSafeTool: virtual public IGenericTool {
public:
  /// InterfaceID
  DeclareInterfaceID(IGenericThreadSafeTool, 1, 0);

  /// Do the action (thread safe).
  virtual void execute() const = 0;

  /// Allow use of IGenericThreadSafeTool as IGenericTool.
  void execute() override final {
    const_cast<const IGenericThreadSafeTool*>(this)->execute();
  }
};

#endif // KERNEL_INORMALIZETOOL_H
