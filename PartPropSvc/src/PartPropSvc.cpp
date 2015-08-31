#ifdef __ICC
// disable icc remark #1572: floating-point equality and inequality comparisons are unreliable
//  TODO: should be removed because come from HepPDT
#pragma warning(disable:1572)
#endif

//Include files
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/PathResolver.h"

#include "PartPropSvc.h"

#include "HepPDT/HeavyIonUnknownID.hh"

#include <fstream>

#include <boost/regex.hpp>
#include <boost/algorithm/string/case_conv.hpp>



//*************************************************************************//

PartPropSvc::PartPropSvc( const std::string& name_, ISvcLocator* svc )
  : base_class( name_, svc ),  m_log(msgSvc(), name())  {

  declareProperty( "InputFile", m_pdtFiles="PDGTABLE.MeV");

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

  static const boost::regex exp{"[[:space:]]*([^[:space:]]+)[[:space:]]*=[[:space:]]*([^[:space:]]+)"};
  static const auto tok_end = boost::sregex_iterator();
  for (auto tok_iter = boost::sregex_iterator(begin(key), end(key), exp);
       tok_iter != tok_end; ++tok_iter)
  {
    const std::string fname = (*tok_iter)[1];

    // see if input file exists in $DATAPATH
    std::string rfile = System::PathResolver::find_file(fname,"DATAPATH");
    if (rfile.empty()) {
      m_log << MSG::ERROR << "Could not find PDT file: \"" << fname
	    << "\" in $DATAPATH" << endmsg;
      return StatusCode::FAILURE;
    }

    // is the file readable?
    std::ifstream pdfile{ rfile };
    if (!pdfile) {
      m_log << MSG::ERROR << "Could not open PDT file: \"" << rfile
	    << "\"" << endmsg;
      return StatusCode::FAILURE;
    }

    std::string val = (*tok_iter)[1];
    std::string VAL = boost::algorithm::to_upper_copy(val);

    // default: no type specified, assume PDG
    if (val == fname) {
      m_log << MSG::INFO << "No table format type specified for \"" << fname
	    << "\". Assuming PDG" << endmsg;
      VAL = "PDG";
    }

    inputFunPtr pF = nullptr;
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

    m_inputs.emplace_back( rfile, pF );

  }


  return status;
}
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
PartPropSvc::finalize() {

  m_pdt.reset();

  if (m_upid_local && m_upid ) {
    m_upid_local = false;
    // This will cause a memory leak, but we can't delete it as the
    // destructor of HepPDT::processUnknownID is protected.
    // We need this though to call reinitialize successfully.
    m_upid = nullptr;
  }

  MsgStream m_log( msgSvc(), name() );
  StatusCode status = Service::finalize();

  if ( status.isSuccess() )
    m_log << MSG::DEBUG << "Service finalised successfully" << endmsg;

  return status;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

PartPropSvc::inputFunPtr
PartPropSvc::parseTableType(const std::string& typ)
{
  static const auto table = { std::make_pair( "PDG"    , &HepPDT::addPDGParticles ),
                              std::make_pair( "PYTHIA" , &HepPDT::addPythiaParticles ),
                              std::make_pair( "EVTGEN" , &HepPDT::addEvtGenParticles ),
                              std::make_pair( "HERWIG" , &HepPDT::addHerwigParticles ),
                              std::make_pair( "ISAJET" , &HepPDT::addIsajetParticles ),
                              std::make_pair( "QQ"     , &HepPDT::addQQParticles ) };
  auto i = std::find_if( std::begin(table), std::end(table), 
                         [&](const std::pair<const char*,inputFunPtr>& p)
                         { return typ == p.first; } );
  if ( i == std::end(table) ) {
    m_log << MSG::ERROR << "Unknown Particle Data file type: \""
	      << typ << "\"" << endmsg;
    throw  std::runtime_error("error parsing particle table type");
  }
  return i->second;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
PartPropSvc::createTable() {

  // use a handler for unknown heavy ions
  if ( !m_upid  ) {
    setUnknownParticleHandler(new HepPDT::HeavyIonUnknownID,
			      "Default Heavy Ion Handler");
    m_upid_local = true;
  }

  m_pdt.reset( new HepPDT::ParticleDataTable(m_upid_name, m_upid) );

  HepPDT::TableBuilder  tb( *m_pdt );

  for (const auto& itr : m_inputs ) {
    const auto& f = itr.first;
    const auto& pF = itr.second;

    m_log << MSG::DEBUG << "Reading PDT file \"" << f << "\""
 	  << endmsg;

    std::ifstream pdfile{ f };
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

  if (!m_pdt ) {
    m_log << MSG::DEBUG << "creating ParticleDataTable" << endmsg;
    if (createTable().isFailure()) {
      m_log << MSG::FATAL << "Could not create ParticleDataTable" << endmsg;
      m_pdt.reset(nullptr);
    }
  }

  return m_pdt.get();
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
PartPropSvc::setUnknownParticleHandler(HepPDT::ProcessUnknownID* puid,
				       const std::string& n) {
  if (m_pdt) {
    m_log << MSG::ERROR << "not setting Unknown Particle Handler \"" << n
	  << "\" as ParticleDataTable already instantiated" << endmsg;
    return;
  }

  m_log << MSG::DEBUG << "setting Unknown Particle Handler \"" << n
	<< "\" at " << puid << endmsg;

  if (m_upid) {
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
