// Framework include files
#include "RunRecordStream.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/MsgStream.h"

// Define the algorithm factory for the standard output data writer
DECLARE_COMPONENT( RunRecordStream )

StatusCode RunRecordStream::finalize()
{
  info() << "Set up File Summary Record" << endmsg;
  StatusCode sc = OutputStream::execute();
  if ( !sc.isSuccess() ) {
    warning() << "Error writing run summary record....." << endmsg;
  }
  return OutputStream::finalize();
}
