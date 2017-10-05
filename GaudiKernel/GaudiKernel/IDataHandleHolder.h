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

class IDataHandleVisitor;
class IDataProviderSvc;

class GAUDI_API IDataHandleHolder : virtual public INamedInterface
{
public:
  virtual std::vector<Gaudi::DataHandle*> inputHandles() const  = 0;
  virtual std::vector<Gaudi::DataHandle*> outputHandles() const = 0;

  virtual const DataObjIDColl& extraInputDeps() const  = 0;
  virtual const DataObjIDColl& extraOutputDeps() const = 0;

  virtual void acceptDHVisitor( IDataHandleVisitor* ) const = 0;

  virtual const DataObjIDColl& inputDataObjs() const  = 0;
  virtual const DataObjIDColl& outputDataObjs() const = 0;

  virtual void addDependency( const DataObjID&, const Gaudi::DataHandle::Mode& ) = 0;

  DeclareInterfaceID( IDataHandleHolder, 1, 0 );

  virtual void declare( Gaudi::DataHandle& )  = 0;
  virtual void renounce( Gaudi::DataHandle& ) = 0;
};

class IDataHandleVisitor
{
public:
  virtual ~IDataHandleVisitor()                  = default;
  virtual void visit( const IDataHandleHolder* ) = 0;
};


/// Work-in-progress rewrite of the DataHandle infrastructure
/// FIXME: Decide how conditions support is going to be added
namespace Gaudi
{
  namespace experimental
  {
    class DataHandle;

    /// The DataHandleHolder implementation will expect the following interface
    /// from the underlying class (Algorithm, AlgTool...).
    ///
    /// One additional contract which cannot be checked cleanly by the compiler
    /// in today's C++ is that the class should inherit from PropertyHolder.
    ///
    struct GAUDI_API IDataHandleHolderReqs
      : public extend_interfaces<IProperty, IStateful>
    {
      /// Provide access to the whiteboard after initialization
      virtual SmartIF<IDataProviderSvc>& eventSvc() const = 0;
    };

    /// The DataHandleHolder implementation will provide the following interface
    struct IDataHandleHolder : public virtual IDataHandleHolderReqs {
      /// Register a data handle as an input of the algorithm
      virtual void registerInput(DataHandle& handle) = 0;

      /// Register a data handle as an output of the algorithm
      virtual void registerOutput(DataHandle& handle) = 0;

      /// Tell which whiteboard keys the algorithm will be reading from
      virtual DataObjIDColl inputKeys() const = 0;

      /// Tell which whiteboard keys the algorithm will be writing to
      virtual DataObjIDColl outputKeys() const = 0;
    };
  }
}

#endif // !GAUDIKERNEL_IDATAHANDLEHOLDER
