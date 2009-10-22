// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IInspectable.h,v 1.3 2006/11/30 15:22:07 dquarrie Exp $
#ifndef GAUDIKERNEL_IINSPECTABLE_H
#define GAUDIKERNEL_IINSPECTABLE_H

// Include Files
#include "GaudiKernel/Kernel.h"

// Forward declaration
class IInspector;
class IInspectable;

/** @class IInspectable IInspectable.h GaudiKernel/IInspectable.h

    Interface definition of an inspectable object.
    The IInspectable interface must be implemented by all objects that are to be inspected

*/
class GAUDI_API IInspectable {
public:

  /// destructor
  virtual ~IInspectable() { }

	/** return FALSE if doesn't accept Inspecetor so that diagnostic
      message can be printed. Calls the inspect member function of
      the Inspector on all the members it wants to inspect.
  */
	virtual bool acceptInspector(IInspector* pI) = 0;
	/** return FALSE if doesn't accept Inspecetor so that diagnostic
      message can be printed. Calls the inspect member function of
      the Inspector on all the members it wants to inspect.
  */
	virtual bool acceptInspector(IInspector* pI) const = 0;
};
#endif  // GAUDIKERNEL_IINSPECTABLE_H
