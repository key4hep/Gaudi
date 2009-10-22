#include "StatusCodeSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/StatusCode.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

DECLARE_SERVICE_FACTORY(StatusCodeSvc)

using namespace std;
//
///////////////////////////////////////////////////////////////////////////
//

StatusCodeSvc::StatusCodeSvc(const std::string& name, ISvcLocator* svc )
  : base_class( name, svc )
{

  declareProperty("Filter",m_pFilter);
  declareProperty("AbortOnError",m_abort=false);
  declareProperty("SuppressCheck", m_suppress=false);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCodeSvc::~StatusCodeSvc() {

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


StatusCode
StatusCodeSvc::initialize() {

  StatusCode sc = Service::initialize();
  if (!sc.isSuccess()) return sc;

  MsgStream log( msgSvc(), name() );
  log << MSG::INFO << "initialize" << endmsg;

  std::vector<std::string>::const_iterator itr;
  for (itr = m_pFilter.value().begin(); itr != m_pFilter.value().end(); ++itr) {
    // we need to do this if someone has gotten to regFnc before initialize
    filterFnc(*itr);

    m_filter.insert(*itr);
  }

  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
StatusCodeSvc::reinitialize() {

  MsgStream log( msgSvc(), name() );
  log << MSG::INFO << "reinitialize" << endmsg;

  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode
StatusCodeSvc::finalize() {

  if (m_dat.size() > 0) {
    MsgStream log( msgSvc(), name() );

    log << MSG::INFO << "listing all unchecked return codes:" << endmsg;

    list();

  }

  return StatusCode::SUCCESS;

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
StatusCodeSvc::regFnc(const std::string& fnc, const std::string& lib) {

  if (m_state == Gaudi::StateMachine::OFFLINE ||
      m_state == Gaudi::StateMachine::CONFIGURED) {
    return;
  }

  if (m_filter.find(fnc) != m_filter.end()) {
    return;
  }

  if (m_abort) {
    MsgStream log( msgSvc(), name() );
    log << MSG::FATAL << "Unchecked StatusCode in " << fnc << " from lib "
	<< lib << endmsg;
    abort();
  }

  string key = fnc + lib;

  map<string,StatCodeDat>::iterator itr = m_dat.find(key);

  if (itr != m_dat.end()) {
    itr->second.count += 1;
  } else {

    int i1 = lib.rfind("/",lib.length());
    string rlib = lib.substr(i1+1,lib.length()-i1-1);

    StatCodeDat dat;
    dat.fnc = fnc;
    dat.lib = rlib;
    dat.count = 1;

    m_dat[key] = dat;
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
StatusCodeSvc::list() const {

  MsgStream log( msgSvc(), name() );
  log << MSG::INFO << endl;

  map<string,StatCodeDat>::const_iterator itr;

#if defined (__GNUC__) && ( __GNUC__ <= 2 )
  std::ostrstream os;
#else
  std::ostringstream os;
#endif

  os << "Num | Function                       | Source Library" << endl;
  os << "----+--------------------------------+-------------------"
     << "-----------------------" << endl;


  for(itr = m_dat.begin(); itr != m_dat.end(); ++itr ) {
    StatCodeDat dat = itr->second;

    os.width(3);
    os.setf(ios_base::right,ios_base::adjustfield);
    os << dat.count;

    os << " | ";
    os.width(30);
    os.setf(ios_base::left,ios_base::adjustfield);
    os << dat.fnc;

    os << " | ";
    os.setf(ios_base::left,ios_base::adjustfield);
    os << dat.lib;

    os << endl;

  }


  log << os.str() << endmsg;

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void
StatusCodeSvc::filterFnc(std::string str) {

  std::map<std::string, StatCodeDat>::iterator itr;
  for (itr = m_dat.begin(); itr != m_dat.end(); ++itr ) {
    if (itr->second.fnc == str) {
      m_dat.erase(itr);
      return;
    }

  }

}
