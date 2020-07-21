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
// Framework include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IDataStoreLeaves.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/MsgStream.h"
#include "OutputStream.h"

class CopyInputStream : public OutputStream {

public:
  /// Standard algorithm Constructor
  CopyInputStream( const std::string& name, ISvcLocator* pSvcLocator );

  /// Collect all objects to be written to the output stream
  StatusCode collectObjects() override;

private:
  Gaudi::Property<std::vector<std::string>> m_tesVetoList{this, "TESVetoList", {}, "names of TES locations to Veto"};

  DataObjectReadHandle<IDataStoreLeaves::LeavesList> m_inputFileLeaves{this, "InputFileLeavesLocation",
                                                                       "/Event/InputFileLeaves"};

protected:
  /// Overridden from the base class (CopyInputStream has always input).
  bool hasInput() const override { return true; }
};

// implementation

// Standard Constructor
CopyInputStream::CopyInputStream( const std::string& name, ISvcLocator* pSvcLocator )
    : OutputStream( name, pSvcLocator ) {
  setProperty( "Preload", false ).ignore();
  setProperty( "PreloadOptItems", false ).ignore();
}

/// Collect all objects to be written to the output stream
StatusCode CopyInputStream::collectObjects() {
  // Get the objects in the same file as the root node
  try {
    // Get all the leaves on the input stream
    const auto& leaves = *m_inputFileLeaves.get();

    // Do we need to veto anything ?
    if ( UNLIKELY( !m_tesVetoList.empty() ) ) {
      // Veto out locations
      IDataStoreLeaves::LeavesList filteredLeaves;
      filteredLeaves.reserve( leaves.size() );
      std::copy_if( leaves.begin(), leaves.end(), std::back_inserter( filteredLeaves ),
                    [&]( IDataStoreLeaves::LeavesList::const_reference i ) {
                      return i && i->registry() &&
                             std::find( m_tesVetoList.begin(), m_tesVetoList.end(), i->registry()->identifier() ) ==
                                 m_tesVetoList.end();
                    } );
      // save the veto'ed list
      m_objects.assign( filteredLeaves.begin(), filteredLeaves.end() );

    } else {
      // no veto'ing, so save everything
      m_objects.assign( leaves.begin(), leaves.end() );
    }
  } catch ( const GaudiException& e ) {
    error() << e.message() << endmsg;
    return StatusCode::FAILURE;
  }

  // Collect the other objects from the transient store (mandatory and optional)
  return OutputStream::collectObjects();
}

DECLARE_COMPONENT( CopyInputStream )
