#ifndef GAUDIKERNEL_IACCELERATOR_H
#define GAUDIKERNEL_IACCELERATOR_H

#include <functional>
#include <vector>

// Framework include files
#include "GaudiKernel/IAlgTask.h"
#include "GaudiKernel/IInterface.h"

/**@class IAccelerator IAccelerator.h GaudiKernel/IAccelerator.h
 *
 *  General interface for an accelerator-based algorithm scheduler.
 *
 *  @author  Illya Shapoval
 *  @version 1.0
 */
class GAUDI_API IAccelerator : virtual public IInterface
{
public:
  /// InterfaceID
  DeclareInterfaceID( IAccelerator, 1, 0 );

  virtual StatusCode push( IAlgTask& task ) = 0;
};
#endif
