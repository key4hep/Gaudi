/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiKernel/RenounceToolInputsVisitor.h"
#include "GaudiKernel/IDataHandleHolder.h"
#include "GaudiKernel/IAlgTool.h"

void RenounceToolInputsVisitor::visit(IAlgTool *alg_tool) {
  IDataHandleHolder *dh_holder = dynamic_cast<IDataHandleHolder *>(alg_tool);
  if (dh_holder) {
    for (Gaudi::DataHandle* input_handle : dh_holder->inputHandles()) {
      for (const std::string &renounce_input : m_renounceKeys) {
        if (input_handle->objKey() == renounce_input) {
          m_logger->renounce(alg_tool->name(), input_handle->objKey());
          dh_holder->renounce(*input_handle);
        }
      }
    }
  }
  std::vector<DataObjID *> erase_list;
  for (const DataObjID &const_obj_id : dh_holder->inputDataObjs() ) {
    DataObjID &obj_id = const_cast<DataObjID &>(const_obj_id);
    for (const std::string &renounce_input : m_renounceKeys) {
      if (obj_id.key()==renounce_input) {
        DataObjID *erase_id =&obj_id;
        erase_list.push_back(erase_id);
      }
    }
  }
  DataObjIDColl &input_objs = const_cast<DataObjIDColl &>(dh_holder->inputDataObjs());
  for (DataObjID *obj : erase_list) {
    m_logger->renounce(alg_tool->name(), obj->key());
    input_objs.erase(*obj);
  }
}
