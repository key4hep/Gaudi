#include "StatusCodeSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/StatusCode.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

using namespace std;
//
///////////////////////////////////////////////////////////////////////////
//
inline void toupper(std::string &s)
{
  std::transform(s.begin(), s.end(), s.begin(),
                 (int(*)(int)) toupper);
}

StatusCodeSvc::StatusCodeSvc(const std::string& name, ISvcLocator* svc )
  : base_class( name, svc )
{

  declareProperty("Filter",m_pFilter);
  declareProperty("AbortOnError",m_abort=false);
  declareProperty("SuppressCheck", m_suppress=false);
  declareProperty("IgnoreDicts",m_dict=true);

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

    string fnc,lib;
    parseFilter(*itr,fnc,lib);

    if (fnc != "") {
      filterFnc(fnc);
      m_filterfnc.insert(fnc);
    }

    if (lib != "") {
      filterLib(lib);
      m_filterlib.insert(lib);
    }

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

  if (m_dict && lib.rfind("Dict.so") == (lib.length()-7) ) {
    return;
  }

  {
    const string rlib = lib.substr(lib.rfind("/") + 1);

    if (m_filterfnc.find(fnc) != m_filterfnc.end() ||
        m_filterlib.find(rlib) != m_filterlib.end() ) {
      return;
    }
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

    const string rlib = lib.substr(lib.rfind("/") + 1);

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
StatusCodeSvc::filterFnc(const std::string& str) {

  std::map<std::string, StatCodeDat>::iterator itr;
  for (itr = m_dat.begin(); itr != m_dat.end(); ++itr ) {
    if (itr->second.fnc == str) {
      m_dat.erase(itr);
      return;
    }

  }

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
StatusCodeSvc::filterLib(const std::string& str) {

  std::map<std::string, StatCodeDat>::iterator itr;
  for (itr = m_dat.begin(); itr != m_dat.end(); ++itr ) {
    if (itr->second.lib == str) {
      m_dat.erase(itr);
      return;
    }

  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
StatusCodeSvc::parseFilter(const string& str, string& fnc, string& lib) {


  string::size_type loc = str.find("=");
  if (loc == std::string::npos) {
    fnc = str;
    lib = "";
  } else {
    string key,val;
    key = str.substr(0,loc);
    val = str.substr(loc+1,str.length()-loc-1);

    toupper(key);

    if (key == "FCN" || key == "FNC") {
      fnc = val;
      lib = "";
    } else if (key == "LIB") {
      fnc = "";
      lib = val;
    } else {
      fnc = "";
      lib = "";

      MsgStream log( msgSvc(), name() );
      log << MSG::WARNING << "ignoring unknown token in Filter: " << str
	  << endmsg;
    }
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

DECLARE_SERVICE_FACTORY(StatusCodeSvc)
