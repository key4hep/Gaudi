/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/IDataHandleHolder.h>
#include <GaudiKernel/RenounceToolInputsVisitor.h>

void RenounceToolInputsVisitor::operator()( IAlgTool* alg_tool ) {
  IDataHandleHolder* dh_holder = dynamic_cast<IDataHandleHolder*>( alg_tool );
  if ( dh_holder ) {
    for ( const DataObjID& id : m_renounceKeys ) {
      if ( dh_holder->renounceInput( id ) ) { m_logger->renounce( alg_tool->name(), id.key() ); }
    }
  }
}
