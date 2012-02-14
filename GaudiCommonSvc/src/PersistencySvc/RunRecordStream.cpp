// $Id: RunRecordStream.cpp,v 1.1 2008/11/04 22:49:25 marcocle Exp $
#define GAUDISVC_PERSISTENCYSVC_OUTPUTFSRSTREAM_CPP

// Framework include files
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/MsgStream.h"
#include "RunRecordStream.h"

// Define the algorithm factory for the standard output data writer
DECLARE_ALGORITHM_FACTORY(RunRecordStream)

StatusCode RunRecordStream::finalize() {
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "Set up File Summary Record" << endmsg;
  StatusCode sc = OutputStream::execute();
  if( !sc.isSuccess() )  {
    log << MSG::WARNING << "Error writing run summary record....." << endmsg;
  }
  return OutputStream::finalize();
}
