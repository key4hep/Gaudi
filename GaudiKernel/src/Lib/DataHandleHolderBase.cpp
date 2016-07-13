#include "GaudiKernel/DataHandleHolderBase.h"
#include "GaudiKernel/DataHandle.h"

// initDataHandleHolderProperties
void DataHandleHolderBase::initDataHandleHolderProperties(PropertyMgr *propertyMgr) {
  //declare Extra input and output properties
  propertyMgr->declareProperty("ExtraInputs",  m_extInputDataObjs, "none");
  propertyMgr->declareProperty("ExtraOutputs", m_extOutputDataObjs, "none");
}

// initDataHandleHolder
void DataHandleHolderBase::initDataHandleHolder() {
  for (auto h : m_inputHandles) h->init();
  for (auto h : m_outputHandles) h->init();
}
