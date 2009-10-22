//Include files
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/PathResolver.h"

#include "PartPropSvc.h"

#include "HepPDT/TableBuilder.hh"

#include <iostream>
#include <cstdlib>
#include <fstream>

// Instantiation of a static factory class used by clients to create
//  instances of this service
DECLARE_SERVICE_FACTORY(PartPropSvc)

//*************************************************************************//

PartPropSvc::PartPropSvc( const std::string& name, ISvcLocator* svc )
  : base_class( name, svc ), m_pdt(0) {

  declareProperty( "InputType", m_inputType="PDG");
  declareProperty( "InputFile", m_pdtFiles);

  if (m_pdtFiles.empty() ) {
    m_pdtFiles.push_back("PDGTABLE.MeV");
  }

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

PartPropSvc::~PartPropSvc() {
  if (m_pdt != 0) {
    delete m_pdt;
    m_pdt = 0;
  }
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode PartPropSvc::initialize() {

  MsgStream log( msgSvc(), name() );
  std::vector<std::string>::const_iterator itr;

  StatusCode status = Service::initialize();

  log << MSG::INFO << "PDT file(s): " << endmsg;
  for (itr=m_pdtFiles.begin(); itr!=m_pdtFiles.end(); ++itr) {
    log << MSG::INFO << "    " << *itr << endmsg;
  }
  log << MSG::INFO << "Type:     " << m_inputType << endmsg;

  if ( status.isFailure() ) {
    log << MSG::ERROR << "Could not initialize main svc" << endmsg;
    return StatusCode::FAILURE;
  }

  bool (*pF)  (std::istream &,
	       HepPDT::TableBuilder &);


  // Determine type of input
  if (m_inputType == "PDG") {
    pF = &HepPDT::addPDGParticles;
  } else if (m_inputType == "Pythia") {
    pF = &HepPDT::addPythiaParticles;
  } else if (m_inputType == "EvtGen") {
    pF = &HepPDT::addEvtGenParticles;
  } else if (m_inputType == "Herwig") {
    pF = &HepPDT::addHerwigParticles;
  } else if (m_inputType == "IsaJet") {
    pF = &HepPDT::addIsajetParticles;
  } else if (m_inputType == "QQ") {
    pF = &HepPDT::addQQParticles;
  } else {
    log << MSG::ERROR << "Unknown Particle Data file type: \""
	<< m_inputType << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  // Make sure we have at least one file
  if (m_pdtFiles.size() == 0) {
    log << MSG::ERROR << "Need at least 1 PDT file" << endmsg;
    log << MSG::ERROR << "Check value of property \"InputFile\"" << endmsg;
    return StatusCode::FAILURE;
  }

  m_pdt = new HepPDT::ParticleDataTable;

  {
    // Construct table builder
    HepPDT::TableBuilder  tb( *m_pdt );

    // read the input
    int good(0);
    for (itr=m_pdtFiles.begin(); itr!=m_pdtFiles.end(); ++itr) {

      std::string rfile = System::PathResolver::find_file(*itr,"DATAPATH");
      if (rfile == "") {
	log << MSG::ERROR << "Could not find PDT file: \"" << *itr
	    << "\" in $DATAPATH" << endmsg;
	continue;
      }

      std::ifstream pdfile( rfile.c_str() );
      if (!pdfile) {
	log << MSG::ERROR << "Could not open PDT file: \"" << rfile
	    << "\"" << endmsg;
	continue;
      }

      if ( ! pF(pdfile,tb) ) {
	log << MSG::ERROR << "Error reading PDT file: \"" << rfile
	    << "\"" << endmsg;
	return StatusCode::FAILURE;
      }
      ++good;
    }
    if (0 == good) {
      log << MSG::ERROR << "Unable to access any PDT file" <<endmsg;
      return StatusCode::FAILURE;
    }

  }   // the tb destructor fills datacol



  return status;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode PartPropSvc::finalize() {

  MsgStream log( msgSvc(), name() );
  StatusCode status = Service::finalize();

  if ( status.isSuccess() )
    log << MSG::INFO << "Service finalised successfully" << endmsg;

  return status;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//
