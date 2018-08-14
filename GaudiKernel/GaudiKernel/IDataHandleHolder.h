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
  inline namespace v1
  {
    class DataHandle;
  }
  namespace v2
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
class GAUDI_API IDataHandleHolder : virtual public extend_interfaces<INamedInterface, IProperty>
{
  using AccessMode = Gaudi::v2::DataHandle::AccessMode;
public:
  DeclareInterfaceID( IDataHandleHolder, 1, 0 );

  /// Provide access to the whiteboard after initialization
  virtual SmartIF<IDataProviderSvc>& eventSvc() const = 0;

  /// Register a data handle of this algorithm/tool
  virtual void registerDataHandle( Gaudi::v2::DataHandle& ) = 0;

  /// Add a data dependency, even after initialization
  virtual void addDataDependency( const DataObjID&, AccessMode ) = 0;

  /// Tell which whiteboard keys the algorithm will be reading or writing
  virtual const DataObjIDColl& dataDependencies( AccessMode ) const = 0;

  /// Declare ownership of a legacy DataHandle
  virtual void declare( Gaudi::v1::DataHandle& ) = 0;

  /// Discard ownership of a legacy DataHandle
  virtual void renounce( Gaudi::v1::DataHandle& ) = 0;

  // ---------------------------------------------------------------------------
  // The following interfaces expose implementation details of the
  // DataHandleHolderBase, and are only meant to be used by said implementation.
  // Please refrain from using them elsewhere.
  // ---------------------------------------------------------------------------

  /// Access the internal array of data dependencies
  virtual const DataObjIDColl* allDataDependencies() const = 0;
};

#endif // !GAUDIKERNEL_IDATAHANDLEHOLDER
