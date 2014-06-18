// Framework include files
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IDataManagerSvc.h"

// Example related include files
#include "MIReadAlg.h"
#include "MIHelpers.h"

// Event Model related classes
#include "GaudiExamples/Event.h"
#include "GaudiExamples/MyTrack.h"

#include "GaudiKernel/System.h"

#include <fstream>


using namespace Gaudi::Examples::MultiInput;

DECLARE_COMPONENT(ReadAlg)

ReadAlg::ReadAlg(const std::string& name, ISvcLocator* pSvcLoc):
  Algorithm(name, pSvcLoc), m_count(0) {
  declareProperty("AddressesFile", m_addressfile,
      "File containing the address details of the extra data.");
}
ReadAlg::~ReadAlg(){}

//--------------------------------------------------------------------
// Initialize
//--------------------------------------------------------------------
StatusCode ReadAlg::initialize() {
  StatusCode sc = Algorithm::initialize();
  if (sc.isFailure()) return sc;

  MsgStream log(msgSvc(), name());
  if (outputLevel() <= MSG::DEBUG)
    log << MSG::DEBUG << "Reading " << m_addressfile << endmsg;
  m_addresses.clear();
  std::ifstream input(m_addressfile.c_str());
  while (input.good()) {
    RootAddressArgs addr;
    input >> addr;
    if (input.eof()) break;
    m_addresses.push_back(addr);
  }
  if (outputLevel() <= MSG::DEBUG)
    log << MSG::DEBUG << "Read " << m_addresses.size() << " addresses" << endmsg;

  m_count = 0;

  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------
// Finalize
//--------------------------------------------------------------------
StatusCode ReadAlg::finalize() {
  return Algorithm::finalize();
}

//--------------------------------------------------------------------
// Execute
//--------------------------------------------------------------------
StatusCode ReadAlg::execute() {
  // This just makes the code below a bit easier to read (and type)
  MsgStream log(msgSvc(), name());

  if (m_count < m_addresses.size()) {
    // register the entry "Extra/Tracks" in the TES so that it is loaded
    // from the other file
    StatusCode sc = eventSvc()->registerObject("Extra", new DataObject);
    if (sc.isFailure()) {
      log << MSG::ERROR << "Cannot add entry 'Extra' to the TES"
          << endmsg;
      return sc;
    }
    sc = SmartIF<IDataManagerSvc>(eventSvc())
        ->registerAddress("Extra/Tracks", make_address(m_addresses[m_count]));
    if (sc.isFailure()) {
      log << MSG::ERROR << "Failed to register the address to the extra data"
          << endmsg;
      return sc;
    }
    ++m_count;
  }

  SmartDataPtr<MyTrackVector> trks1(eventSvc(), "Tracks");
  SmartDataPtr<MyTrackVector> trks2(eventSvc(), "Extra/Tracks");

  if (trks1)
    log << MSG::INFO << "Base event tracks: " << trks1->size() << endmsg;
  else
    log << MSG::WARNING << "No tracks container in base event" << endmsg;
  if (trks2)
    log << MSG::INFO << "Extra event tracks: " << trks2->size() << endmsg;
  else
      log << MSG::WARNING << "No tracks container in extra event" << endmsg;

  return StatusCode::SUCCESS;
}
