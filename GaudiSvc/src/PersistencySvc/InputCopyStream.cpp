// $Id: InputCopyStream.cpp,v 1.2 2006/01/10 20:09:27 hmd Exp $
#define GAUDISVC_PERSISTENCYSVC_INPUTCOPYSTREAM_CPP

// Framework include files
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataStoreLeaves.h"
#include "GaudiKernel/MsgStream.h"
#include "InputCopyStream.h"

// Define the algorithm factory for the standard output data writer
DECLARE_ALGORITHM_FACTORY(InputCopyStream)

// Standard Constructor
InputCopyStream::InputCopyStream(const std::string& name, ISvcLocator* pSvcLocator)
 : OutputStream(name, pSvcLocator), m_leavesTool(0)
{
  m_doPreLoad      = false;
  m_doPreLoadOpt   = false;
}

// Standard Destructor
InputCopyStream::~InputCopyStream()   {
}

// Initialize the instance.
StatusCode InputCopyStream::initialize() {
  StatusCode sc = OutputStream::initialize();
  if (sc.isFailure()) return sc;

  sc = toolSvc()->retrieveTool("DataSvcFileEntriesTool", "InputCopyStreamTool",
                               m_leavesTool);
  if (sc.isFailure()) return sc;

  return StatusCode::SUCCESS;
}

// Finalize the instance.
StatusCode InputCopyStream::finalize() {
  toolSvc()->releaseTool(m_leavesTool).ignore();
  m_leavesTool = 0;

  return OutputStream::finalize();
}

/// Collect all objects to be written to the output stream
StatusCode InputCopyStream::collectObjects()   {
  MsgStream log(msgSvc(), name());

  // Get the objects in the same file as the root node
  try {
    const IDataStoreLeaves::LeavesList & leaves = m_leavesTool->leaves();
    m_objects.assign(leaves.begin(), leaves.end());
  } catch (GaudiException &e) {
    log << MSG::ERROR << e.message() << endmsg;
    return StatusCode::FAILURE;
  }

  // Collect the other objects from the transient store (mandatory and optional)
  return OutputStream::collectObjects();
}
