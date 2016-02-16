#ifndef GAUDIKERNEL_IDATAHANDLEHOLDER
#define GAUDIKERNEL_IDATAHANDLEHOLDER 1

#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/DataObjID.h"

#include <vector>
#include <set>

namespace Gaudi {
  class DataHandle;
}

class IDataHandleVisitor;

class GAUDI_API IDataHandleHolder : virtual public INamedInterface {
public:
   virtual ~IDataHandleHolder() {};

   virtual std::vector<Gaudi::DataHandle*> inputHandles() const = 0;
   virtual std::vector<Gaudi::DataHandle*> outputHandles() const = 0;

   virtual const DataObjIDColl& extraInputDeps() const = 0;
   virtual const DataObjIDColl& extraOutputDeps() const = 0;

   virtual void acceptDHVisitor(IDataHandleVisitor*) const = 0;

   virtual void commitHandles() = 0;

   DeclareInterfaceID(IDataHandleHolder,1,0);

 protected:
   virtual void declareInput(Gaudi::DataHandle*) = 0;
   virtual void declareOutput(Gaudi::DataHandle*) = 0;

};


class IDataHandleVisitor {
public:
  virtual void visit(const IDataHandleHolder*) = 0;
};

#endif // !GAUDIKERNEL_IDATAHANDLEHOLDER
