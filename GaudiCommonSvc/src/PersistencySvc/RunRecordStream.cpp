// Framework include files
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/MsgStream.h"
#include "RunRecordStream.h"

// Define the algorithm factory for the standard output data writer
DECLARE_COMPONENT(RunRecordStream)

StatusCode RunRecordStream::finalize() {
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "Set up File Summary Record" << endmsg;
  StatusCode sc = OutputStream::execute();
  if( !sc.isSuccess() )  {
    log << MSG::WARNING << "Error writing run summary record....." << endmsg;
  }
  return OutputStream::finalize();
}
