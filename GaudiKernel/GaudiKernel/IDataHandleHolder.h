#ifndef GAUDIKERNEL_IDATAHANDLEHOLDER
#define GAUDIKERNEL_IDATAHANDLEHOLDER 1

#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IStateful.h"
#include "GaudiKernel/SmartIF.h"

#include <set>
#include <vector>

class IDataProviderSvc;

class GAUDI_API IDataHandleHolder : virtual public INamedInterface
{
public:

  DeclareInterfaceID( IDataHandleHolder, 1, 0 );

  virtual void declare( Gaudi::DataHandle& )  = 0;
  virtual void renounce( Gaudi::DataHandle& ) = 0;
};


/// Work-in-progress rewrite of the DataHandle infrastructure
namespace Gaudi
{
  namespace experimental
  {
    class DataHandle;

    /// The DataHandleHolder implementation will expect the following interface
    /// from the underlying base class (an Algorithm, AlgTool...).
    ///
    /// One additional interface contract that sadly cannot be fully expressed
    /// in code is that the base class should inherit from the PropertyHolder
    /// class template, so that we can declare DataHandles as properties of the
    /// host Algorithm or Tool using the declareProperty mechanism.
    ///
    /// As the ability of current C++ to express interface requirements on
    /// templated classes and methods is very limited, we cannot elegantly check
    /// that the base class has a declareProperty<>() member function template,
    /// nor that it is is an instance of the PropertyHolder template. All we can
    /// check for is the IProperty interface, of which PropertyHolder is only
    /// one possible implementation...
    ///
    struct GAUDI_API IDataHandleHolderReqs
      : virtual public extend_interfaces<IProperty>
    {
      DeclareInterfaceID( IDataHandleHolderReqs, 1, 0 );

      /// Provide access to the whiteboard after initialization
      virtual SmartIF<IDataProviderSvc>& eventSvc() const = 0;
    };


    /// The DataHandleHolder implementation will provide the following interface
    ///
    /// This interface, which only has a single implementation, solely exists
    /// in order to work around various C++ limitations pertaining to templates,
    /// forward declarations, and circular dependencies between classes. If you
    /// can call the relevant DataHandleHolder methods directly, go for it.
    ///
    struct GAUDI_API IDataHandleHolder
      : virtual public extend_interfaces<IDataHandleHolderReqs>
    {
      DeclareInterfaceID( IDataHandleHolder, 1, 0 );

      /// Register a data handle as an event data input of the algorithm
      virtual void registerEventInput(DataHandle& handle) = 0;

      /// Register a data handle as an event data output of the algorithm
      virtual void registerEventOutput(DataHandle& handle) = 0;

      /// Add an event data output dynamically at run time
      ///
      /// DataHandles are the preferred way to declare statically known data
      /// dependencies. However, there are cases in which an Algorithm's data
      /// outputs are only known at run time, typically when data is loaded on
      /// demand from a file or database. In this case, this method should be
      /// used to declare those dynamic data dependencies.
      ///
      virtual void addDynamicEventOutput(const DataObjID& key) = 0;

      /// Tell which event data keys the algorithm will be reading from
      virtual const DataObjIDColl& eventInputKeys() const = 0;

      /// Tell which event data keys the algorithm will be writing to
      virtual const DataObjIDColl& eventOutputKeys() const = 0;
    };
  }
}

#endif // !GAUDIKERNEL_IDATAHANDLEHOLDER
