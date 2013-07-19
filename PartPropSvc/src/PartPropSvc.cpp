#ifdef __ICC
// disable icc remark #1572: floating-point equality and inequality comparisons are unreliable
//  TODO: should be removed because come from HepPDT
#pragma warning(disable:1572)
#endif

//Include files
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/PathResolver.h"
#include "GaudiKernel/Tokenizer.h"

#include "PartPropSvc.h"

//#include "HepPDT/TableBuilder.hh"
#include "HepPDT/HeavyIonUnknownID.hh"

//#include <iostream>
#include <cstdlib>
#include <fstream>

using namespace std;

inline void toupper(std::string &s)
{
  std::transform(s.begin(), s.end(), s.begin(),
                 (int(*)(int)) toupper);
}

//*************************************************************************//

PartPropSvc::PartPropSvc( const std::string& name, ISvcLocator* svc )
  : base_class( name, svc ),  m_upid(0), m_pdt(0), m_log(msgSvc(), name),
    m_upid_local(false) {

  declareProperty( "InputFile", m_pdtFiles="PDGTABLE.MeV");

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

PartPropSvc::~PartPropSvc() {
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
PartPropSvc::initialize() {

  StatusCode status = Service::initialize();
  m_log.setLevel( m_outputLevel.value() );

  if ( status.isFailure() ) {
    m_log << MSG::ERROR << "Could not initialize main svc" << endmsg;
    return StatusCode::FAILURE;
  }


  std::string key = m_pdtFiles.value();

  Tokenizer tok(true);

  tok.analyse( key, " ", "", "", "=", "", "");

  for ( Tokenizer::Items::iterator i = tok.items().begin();
	i != tok.items().end(); i++)    {
    const std::string& fname = (*i).tag();

    // see if input file exists in $DATAPATH
    std::string rfile = System::PathResolver::find_file(fname,"DATAPATH");
    if (rfile == "") {
      m_log << MSG::ERROR << "Could not find PDT file: \"" << fname
	    << "\" in $DATAPATH" << endmsg;
      return StatusCode::FAILURE;
    }


    // is the file readable?
    std::ifstream pdfile( rfile.c_str() );
    if (!pdfile) {
      m_log << MSG::ERROR << "Could not open PDT file: \"" << rfile
	    << "\"" << endmsg;
      return StatusCode::FAILURE;
    }

    std::string val,VAL;
    val = (*i).value();
    VAL = val;
    toupper(VAL);

    // default: no type specified, assume PDG
    if (val == fname) {
      m_log << MSG::INFO << "No table format type specified for \"" << fname
	    << "\". Assuming PDG" << endmsg;
      VAL = "PDG";
    }

    bool (*pF)  (std::istream &,
		 HepPDT::TableBuilder &);
    try {
      pF = parseTableType(VAL);
    } catch (...) {
      m_log << MSG::ERROR
	    << "Could not determine Particle Property table type: \""
	    << val << "\" for file \"" << fname << "\"" << endmsg;
      return StatusCode::FAILURE;
    }

    m_log << MSG::DEBUG << "Adding PDT file \"" << rfile << "\" type "
	  << VAL << endmsg;

    m_inputs.push_back( make_pair( rfile, pF ) );

  }


  return status;
}
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
PartPropSvc::reinitialize() {

  return StatusCode::SUCCESS;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
PartPropSvc::finalize() {

  if (m_pdt != 0) {
    delete m_pdt;
    m_pdt = 0;
  }

  if (m_upid_local && m_upid != 0) {
    m_upid_local = false;
    // This will cause a memory leak, but we can't delete it as the
    // destructor of HepPDT::processUnknownID is protected.
    // We need this though to call reinitialize successfully.
    m_upid = 0;
  }

  MsgStream m_log( msgSvc(), name() );
  StatusCode status = Service::finalize();

  if ( status.isSuccess() )
    m_log << MSG::DEBUG << "Service finalised successfully" << endmsg;

  return status;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

bool
(*PartPropSvc::parseTableType(std::string& typ))(std::istream&,
						 HepPDT::TableBuilder&) {

  bool (*pF)  (std::istream &,
	       HepPDT::TableBuilder &);

  if (typ == "PDG") {
    pF = &HepPDT::addPDGParticles;
  } else if (typ == "PYTHIA") {
    pF = &HepPDT::addPythiaParticles;
  } else if (typ == "EVTGEN") {
    pF = &HepPDT::addEvtGenParticles;
  } else if (typ == "HERWIG") {
    pF = &HepPDT::addHerwigParticles;
  } else if (typ == "ISAJET") {
    pF = &HepPDT::addIsajetParticles;
  } else if (typ == "QQ") {
    pF = &HepPDT::addQQParticles;
  } else {
    m_log << MSG::ERROR << "Unknown Particle Data file type: \""
	<< typ << "\"" << endmsg;
     throw( std::runtime_error("error parsing particle table type") );
  }

  return pF;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
PartPropSvc::createTable() {

  // use a handler for unknown heavy ions
  if ( m_upid == 0 ) {
    setUnknownParticleHandler(new HepPDT::HeavyIonUnknownID,
			      "Default Heavy Ion Handler");
    m_upid_local = true;
  }

  m_pdt = new HepPDT::ParticleDataTable(m_upid_name, m_upid);

  HepPDT::TableBuilder  tb( *m_pdt );

  std::vector< std::pair<std::string,
    bool(*) (std::istream&,HepPDT::TableBuilder&)> >::const_iterator itr;
  for (itr = m_inputs.begin(); itr != m_inputs.end(); ++itr) {
    string f = itr->first;
    bool (*pF) (std::istream&,HepPDT::TableBuilder&) = itr->second;

    m_log << MSG::DEBUG << "Reading PDT file \"" << f << "\""
 	  << endmsg;

    std::ifstream pdfile( f.c_str() );
    // build a table from the file
    if ( ! pF(pdfile,tb) ) {
      m_log << MSG::ERROR << "Error reading PDT file: \"" << f
	    << "\"" << endmsg;
      return StatusCode::FAILURE;
    }

  }

  return StatusCode::SUCCESS;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

HepPDT::ParticleDataTable*
PartPropSvc::PDT() {

  if (m_pdt == 0) {
    m_log << MSG::DEBUG << "creating ParticleDataTable" << endmsg;
    if (createTable().isFailure()) {
      m_log << MSG::FATAL << "Could not create ParticleDataTable" << endmsg;
      m_pdt = 0;
    }
  }

  return m_pdt;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
PartPropSvc::setUnknownParticleHandler(HepPDT::ProcessUnknownID* puid,
				       const std::string& n) {
  if (m_pdt != 0) {
    m_log << MSG::ERROR << "not setting Unknown Particle Handler \"" << n
	  << "\" as ParticleDataTable already instantiated" << endmsg;
    return;
  }

  m_log << MSG::DEBUG << "setting Unknown Particle Handler \"" << n
	<< "\" at " << puid << endmsg;

  if (m_upid != 0) {
    m_log << MSG::WARNING
	  << "overriding previously selected Unknown Particle Handler \""
	  << m_upid_name << "\" with \"" << n << "\"" << endmsg;
  }

  m_upid = puid;
  m_upid_name = n;

}

// Instantiation of a static factory class used by clients to create
//  instances of this service
DECLARE_COMPONENT(PartPropSvc)
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//
