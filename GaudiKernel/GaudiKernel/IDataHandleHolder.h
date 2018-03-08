#ifndef GAUDIKERNEL_IDATAHANDLEHOLDER
#define GAUDIKERNEL_IDATAHANDLEHOLDER 1

#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/INamedInterface.h"

#include <set>
#include <vector>

class IDataHandleVisitor;

class GAUDI_API IDataHandleHolder : virtual public INamedInterface
{
public:
  virtual std::vector<Gaudi::DataHandle*> inputHandles() const  = 0;
  virtual std::vector<Gaudi::DataHandle*> outputHandles() const = 0;

  virtual const DataObjIDColl& extraInputDeps() const  = 0;
  virtual const DataObjIDColl& extraOutputDeps() const = 0;

  virtual void acceptDHVisitor( IDataHandleVisitor* ) const = 0;

#if defined( GAUDI_V30_DATAHANDLE_COMMIT )
  virtual void commitHandles() = 0;
#endif

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

#endif // !GAUDIKERNEL_IDATAHANDLEHOLDER
