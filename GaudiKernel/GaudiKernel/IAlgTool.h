#ifndef GAUDIKERNEL_IALGTOOL_H
#define GAUDIKERNEL_IALGTOOL_H

// Include files
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/IStateful.h"
#include <string>

// Forward declarations
class IAlgorithm;
class DataObjectDescriptorCollection;

/** @class IAlgTool IAlgTool.h GaudiKernel/IAlgTool.h

    The interface implemented by the AlgTool base class.
    Concrete tools, derived from the AlgTool based class are
    controlled via this interface.

    @author Gloria Corti
    @author Pere Mato
    @date 15/11/01 version 2 introduced
*/
class GAUDI_API IAlgTool: virtual public extend_interfaces2<INamedInterface,IStateful> {
public:
  /// InterfaceID
  DeclareInterfaceID(IAlgTool,4,0);

  /// Virtual destructor
  ~IAlgTool() override = default;

  /// The type of an AlgTool, meaning the concrete AlgTool class.
  virtual const std::string&  type() const = 0;

  /** The parent of the concrete AlgTool. It can be an Algorithm
      or a Service. A common AlgTool has the ToolSvc as parent.
  */
  virtual const IInterface*   parent() const = 0;

  virtual const DataObjectDescriptorCollection & inputDataObjects() const = 0;
  virtual const DataObjectDescriptorCollection & outputDataObjects() const = 0;

  /** Initialization of the Tool. This method is called typically
   *  by the ToolSvc. It allows to complete  the initialization that
   *  could not be done with a simply a constructor.
   */
  virtual StatusCode sysInitialize() = 0;

  /** Initialization of the Tool. This method is called typically
   *  by the ToolSvc. It allows to complete  the initialization that
   *  could not be done with a simply a constructor.
   */
  virtual StatusCode sysStart() = 0;

  /** Initialization of the Tool. This method is called typically
   *  by the ToolSvc. It allows to complete  the initialization that
   *  could not be done with a simply a constructor.
   */
  virtual StatusCode sysStop() = 0;

  /** Finalization of the Tool. This method is called typically
   *  by the ToolSvc. For completeless this method is called before
   *  the tool is destructed.
   */
  virtual StatusCode sysFinalize() = 0;

  /** Initialization of the Tool. This method is called typically
   *  by the ToolSvc. It allows to complete  the initialization that
   *  could not be done with a simply a constructor.
   */
  virtual StatusCode sysReinitialize() = 0;

  /** Initialization of the Tool. This method is called typically
   *  by the ToolSvc. It allows to complete  the initialization that
   *  could not be done with a simply a constructor.
   */
  virtual StatusCode sysRestart() = 0;

};

#endif  // GAUDIKERNEL_IALGTOOL_H
