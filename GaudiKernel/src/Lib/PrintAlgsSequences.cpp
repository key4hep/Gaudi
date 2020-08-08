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
#include "Gaudi/Property.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/MsgStream.h"

namespace {
  /// Recursive function to print the algorithm name and its sub algorithms
  void printAlgsSequences( SmartIF<IAlgManager>& algmgr, const std::string& algname, MsgStream& log, int indent ) {
    using Gaudi::Utils::TypeNameString;
    log << MSG::ALWAYS;
    for ( int i = 0; i < indent; ++i ) log << "     ";
    log << algname << endmsg;
    auto prop = algmgr->algorithm<IProperty>( algname, false );
    if ( prop ) {
      // Try to get the property Members
      Gaudi::Property<std::vector<std::string>> p( "Members", {} );
      if ( prop->getProperty( &p ).isSuccess() ) {
        for ( auto& subalgname : p.value() ) { printAlgsSequences( algmgr, subalgname, log, indent + 1 ); }
      }
    } else {
      log << MSG::WARNING << "Cannot get properties of " << algname << endmsg;
    }
  }
  /// Helper function to print the sequence of algorithms that have been loaded.
  void printAlgsSequences( IInterface* app ) {
    auto prop   = SmartIF<IProperty>( app );
    auto algmgr = SmartIF<IAlgManager>( app );
    auto msgsvc = SmartIF<IMessageSvc>( app );
    if ( !prop || !algmgr ) return;
    Gaudi::Property<std::vector<std::string>> topalg( "TopAlg", {} );
    if ( prop->getProperty( &topalg ).isSuccess() ) {
      MsgStream log( msgsvc, "ApplicationMgr" );
      log << MSG::ALWAYS << "****************************** Algorithm Sequence ****************************" << endmsg;
      for ( auto& algname : topalg.value() ) { printAlgsSequences( algmgr, algname, log, 0 ); }
      log << MSG::ALWAYS << "******************************************************************************" << endmsg;
    }
  }
} // namespace

#ifdef GAUDI_HASCLASSVISIBILITY
#  pragma GCC visibility push( default )
#endif
extern "C" {
/// Helper to call printAlgsSequences from Pyhton ctypes.
void py_helper_printAlgsSequences( IInterface* app ) { printAlgsSequences( app ); }
}
#ifdef GAUDI_HASCLASSVISIBILITY
#  pragma GCC visibility pop
#endif
