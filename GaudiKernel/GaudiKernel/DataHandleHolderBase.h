#ifndef GAUDIKERNEL_DATAHANDLEHOLDERBASE
#define GAUDIKERNEL_DATAHANDLEHOLDERBASE 1

#include "GaudiKernel/IDataHandleHolder.h"
#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/GaudiException.h"

#include <unordered_set>
#include <algorithm>

class PropertyMgr;

namespace {
    template <typename Container>
    std::vector<Gaudi::DataHandle*> handles(Container& c, Gaudi::DataHandle::Mode mode) {
      std::vector<Gaudi::DataHandle*> h;
      std::copy_if( std::begin(c), std::end(c),
                    std::back_inserter(h),
                    [&](const Gaudi::DataHandle* hndl) -> bool
                    { return hndl->mode() & mode ; }  );
      return h;
    }
}

class GAUDI_API DataHandleHolderBase : virtual public IDataHandleHolder {
 public:

  std::vector<Gaudi::DataHandle*> inputHandles() const override {
      return handles( m_handles, Gaudi::DataHandle::Reader );
  }
  std::vector<Gaudi::DataHandle*> outputHandles() const override {
      return handles( m_handles, Gaudi::DataHandle::Writer );
  }

  virtual const DataObjIDColl& extraInputDeps() const override {
    return m_extInputDataObjs;
  }
  virtual const DataObjIDColl& extraOutputDeps() const override {
    return m_extOutputDataObjs;
  }

  void declare(Gaudi::DataHandle& handle) override {
    // the hndl owner is set in the handl c'tor -- how else can it get here?
    // if (!handle.owner()) handle.setOwner(this);

   if (handle.owner()!=this) {
     throw GaudiException("Attempt to declare foreign handle with algorithm!",
                           name(), StatusCode::FAILURE);
   }

    m_handles.insert(&handle);
  }

  void renounce(Gaudi::DataHandle& handle) override {
   if (handle.owner()!=this) {
     throw GaudiException("Attempt to renounce foreign handle with algorithm!",
                           name(), StatusCode::FAILURE);
   }
   m_handles.erase(&handle);

  }

 protected:

  /// initProperties - to be called by the constructor of the children
  /// once the PropertyManager is available
  void initDataHandleHolderProperties(PropertyMgr *propertyMgr);

  /// initializes all handles - called by the sysInitialize method
  /// of any descendant of this
  virtual void initDataHandleHolder();

 private:

  std::unordered_set<Gaudi::DataHandle*> m_handles;
  DataObjIDColl m_extInputDataObjs, m_extOutputDataObjs;

};

#endif // !GAUDIKERNEL_DATAHANDLEHOLDERBASE
