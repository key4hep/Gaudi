// Framework include files
#include "RecordStream.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/MsgStream.h"

// Define the algorithm factory for the standard output data writer
DECLARE_COMPONENT( RecordStream )

// Standard Constructor
RecordStream::RecordStream( const std::string& name, ISvcLocator* pSvcLocator ) : OutputStream( name, pSvcLocator ) {
  /// in the baseclass, always fire the incidents by default
  /// in RecordStream this will be set to false, and configurable
  declareProperty( "FireIncidents", m_fireIncidents = false, "if to fire incidents for writing opening/closing etc" );
}

StatusCode RecordStream::finalize() {
  info() << "Set up File Summary Record" << endmsg;
  if ( !m_fireIncidents && msgLevel( MSG::VERBOSE ) ) verbose() << "will not fire incidents" << endmsg;
  StatusCode sc = OutputStream::execute();
  if ( !sc.isSuccess() ) { warning() << "Error writing run summary record....." << endmsg; }
  return OutputStream::finalize();
}
