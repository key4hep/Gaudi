#ifndef GAUDIKERNEL_IDATAHANDLEHOLDER
#define GAUDIKERNEL_IDATAHANDLEHOLDER 1

#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/SmartIF.h"

class IDataProviderSvc;

namespace Gaudi
{
  namespace experimental
  {
    class DataHandle;
  }
}

/// Entity which holds DataHandles and can track the associated data
/// dependencies for the Scheduler
///
/// To avoid code duplication between Algorithms and AlgTools, the handle and
/// dependency management functionality has been factored out in a base class
/// template called DataHandleHolderBase. There is only one implementation of
/// this functionality, but due to various C++ limitations pertaining to
/// templates, forward declarations, and circular dependencies, an abstract
/// interface is nevertheless needed in some use cases. Please refer to the
/// DataHandleHolderBase implementation for the full method documentation.
///
/// Users of the implementation (Algorithm and AlgTools) are expected to fulfill
/// the INamedInterface requirement, to inherit from PropertyHolder, and to
/// provide access to the event store. This requirement is encoded in code to
/// the extent which C++ will cleanly allow.
///
struct GAUDI_API IDataHandleHolder : virtual public extend_interfaces<INamedInterface, IProperty> {
  DeclareInterfaceID( IDataHandleHolder, 1, 0 );

  /// Provide access to the whiteboard after initialization
  virtual SmartIF<IDataProviderSvc>& eventSvc() const = 0;

  /// Register a data handle as an event data input of the algorithm
  virtual void registerEventInput( Gaudi::experimental::DataHandle& ) = 0;

  /// Register a data handle as an event data output of the algorithm
  virtual void registerEventOutput( Gaudi::experimental::DataHandle& ) = 0;

  /// Add an event data input dynamically at run time
  virtual void addDynamicEventInput( const DataObjID& ) = 0;

  /// Add an event data output dynamically at run time
  virtual void addDynamicEventOutput( const DataObjID& ) = 0;

  /// Tell which event data keys the algorithm will be reading from
  virtual const DataObjIDColl& eventInputKeys() const = 0;

  /// Tell which event data keys the algorithm will be writing to
  virtual const DataObjIDColl& eventOutputKeys() const = 0;

  /// Declare ownership of a legacy DataHandle
  virtual void declare( Gaudi::DataHandle& ) = 0;

  /// Discard ownership of a legacy DataHandle
  virtual void renounce( Gaudi::DataHandle& ) = 0;
};

#endif // !GAUDIKERNEL_IDATAHANDLEHOLDER
