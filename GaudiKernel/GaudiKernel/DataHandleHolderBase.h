#ifndef GAUDIKERNEL_DATAHANDLEHOLDERBASE
#define GAUDIKERNEL_DATAHANDLEHOLDERBASE 1

#include "GaudiKernel/IDataHandleHolder.h"

#include <vector>

class PropertyMgr;

class GAUDI_API DataHandleHolderBase : virtual public IDataHandleHolder {
 public:
  virtual ~DataHandleHolderBase() {};

  virtual std::vector<Gaudi::DataHandle*> inputHandles() const override {
    return m_inputHandles;
  }
  virtual std::vector<Gaudi::DataHandle*> outputHandles() const override {
    return m_outputHandles;
  }

  virtual const DataObjIDColl& extraInputDeps() const override {
    return m_extInputDataObjs;
  }
  virtual const DataObjIDColl& extraOutputDeps() const override {
    return m_extOutputDataObjs;
  }

 protected:

  /// initProperties - to be called by the constructor of the children
  /// once the PropertyManager is available
  void initDataHandleHolderProperties(PropertyMgr *propertyMgr);

  /// initializes all handles, to ba called by the sysInitialize method
  /// of any descendant of this
  virtual void initDataHandleHolder();

  virtual void declareInput(Gaudi::DataHandle* im) override {
    m_inputHandles.push_back(im);
  }

  virtual void declareOutput(Gaudi::DataHandle* im) override {
    m_outputHandles.push_back(im);
  }

 private:

  std::vector<Gaudi::DataHandle*> m_inputHandles, m_outputHandles;
  DataObjIDColl m_extInputDataObjs, m_extOutputDataObjs;

};

#endif // !GAUDIKERNEL_DATAHANDLEHOLDERBASE
