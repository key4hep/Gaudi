///////////////////////// -*- C++ -*- /////////////////////////////
// IoComponentMgr.cxx
// Implementation file for class IoComponentMgr
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////

// GaudiMP includes
#include "IoComponentMgr.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/FileIncident.h"

#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/ServiceHandle.h"
#include <boost/filesystem.hpp>

// STL includes

// FrameWork includes
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/Property.h"

#define ON_DEBUG if (UNLIKELY(outputLevel() <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(outputLevel() <= MSG::VERBOSE))

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

DECLARE_SERVICE_FACTORY(IoComponentMgr)

using namespace std;


std::ostream& 
operator<< ( std::ostream& os, const IIoComponentMgr::IoMode::Type& m) {
  switch (m) {
  case IIoComponentMgr::IoMode::READ :  os << "READ"; break;
  case IIoComponentMgr::IoMode::WRITE : os << "WRITE"; break;
  case IIoComponentMgr::IoMode::RW :    os << "RW"; break;
  default:
    os << "INVALID"; break;
  }
    
  return os;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IoComponentMgr::IoComponentMgr( const std::string& name,
				ISvcLocator* svc )
: base_class(name,svc), m_log(msgSvc(), name )
{
  //
  // Property declaration
  //
  //declareProperty( "Property", m_nProperty );

//   declareProperty ("Registry",
// 		   m_dict_location = "GaudiMP.IoRegistry.registry",
// 		   "Location of the python dictionary holding the "
// 		   "associations: \n"
// 		   " {component-name:{ 'old-fname' : ['io','new-fname'] }}\n"
// 		   "\nSyntax: <python-module>.<python-module>.<fct-name> \n"
// 		   " where fct-name is a function returning the wanted "
// 		   " dictionary.");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IoComponentMgr::~IoComponentMgr()
{
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode 
IoComponentMgr::initialize() {
  m_log << MSG::DEBUG << "--> initialize()" << endmsg;

  if ( Service::initialize().isFailure() ) {
    m_log << MSG::ERROR << "Unable to initialize Service base class" << endmsg;
    return StatusCode::FAILURE;
  }
  m_log.setLevel( m_outputLevel.value() );

  IIncidentSvc* p_incSvc(0);

  if (service("IncidentSvc", p_incSvc, true).isFailure()) {
    m_log << MSG::ERROR << "unable to get the IncidentSvc" << endmsg;
      return StatusCode::FAILURE;
  } else {
    p_incSvc->addListener( this, IncidentType::BeginOutputFile, 100, true);
    p_incSvc->addListener( this, IncidentType::BeginInputFile, 100, true);
  }

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode 
IoComponentMgr::finalize() {
  m_log << MSG::DEBUG << "--> finalize()" << endmsg;

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** @brief: check if the registry contains a given @c IIoComponent
   */
bool
IoComponentMgr::io_hasitem (IIoComponent* iocomponent) const {
  DEBMSG << "--> io_hasitem()" << endmsg;
  if ( 0 == iocomponent ) {
    return false;
  }
  const std::string& ioname = iocomponent->name();
  IoRegistry_t::const_iterator io = m_ioregistry.find (ioname);
  return io != m_ioregistry.end();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** @brief: check if the registry contains a given @c IIoComponent and
 *          that component had @param `fname` as a filename
 */
bool
IoComponentMgr::io_contains (IIoComponent* iocomponent,
			     const std::string& fname) const
{
  m_log << MSG::DEBUG << "--> io_contains()" << endmsg;
  if ( 0 == iocomponent ) {
    return false;
  }
  const std::string& ioname = iocomponent->name();

  DEBMSG << "io_contains:  c: " << ioname << " f: " << fname << endmsg;

  pair<iodITR,iodITR> fit = m_cdict.equal_range(iocomponent);
  if (fit.first == fit.second) {
    return false;
  } else {
    iodITR it;
    for (it=fit.first; it != fit.second; ++it) {
      IoComponentEntry ioe = it->second;
      DEBMSG << "   " << ioe << endmsg;
      if (ioe.m_oldfname == "") {
	m_log << MSG::ERROR << "IIoComponent " << ioname 
	      << "  has empty old filename" << endmsg;
    return false;
      } else if (ioe.m_oldfname == fname) {
	return true;
      }
    } 
  }

  return false;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** @brief: allow a @c IIoComponent to register itself with this
 *          manager so appropriate actions can be taken when e.g.
 *          a @c fork(2) has been issued (this is usually handled
 *          by calling @c IIoComponent::io_reinit on every registered
 *          component)
 */
StatusCode
IoComponentMgr::io_register (IIoComponent* iocomponent)
{
  if ( 0 == iocomponent ) {
    m_log << MSG::ERROR
	  << "io_register (component) received a NULL pointer !" << endmsg;
    return StatusCode::FAILURE;
  }
  const std::string& ioname = iocomponent->name();
  DEBMSG << "--> io_register(" << ioname << ")" << endmsg;
  IoRegistry_t::iterator itr = m_ioregistry.find (ioname);
  if ( itr == m_ioregistry.end() ) {
    DEBMSG << "    registering IoComponent \"" << ioname << "\"" << endmsg;
    iocomponent->addRef(); // ownership...
    m_ioregistry[ioname] = iocomponent;
    m_iostack.push_back (iocomponent);
  } else {
    m_log << MSG::INFO << "IoComponent[" << iocomponent->name()
	  <<"] already registered @" << (void*)itr->second << endmsg;
  }
  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** @brief: allow a @c IIoComponent to register itself with this
 *          manager so appropriate actions can be taken when e.g.
 *          a @c fork(2) has been issued (this is usually handled
 *          by calling @c IIoComponent::io_reinit on every registered
 *          component)
 */
StatusCode
IoComponentMgr::io_register (IIoComponent* iocomponent,
			     IIoComponentMgr::IoMode::Type iomode,
			     const std::string& fname,
			     const std::string& pfn)
{
  if ( 0 == iocomponent ) {
    return StatusCode::FAILURE;
  }
  const std::string& ioname = iocomponent->name();

  DEBMSG << "--> io_register(" << ioname << ","
	 << ( (iomode== IIoComponentMgr::IoMode::READ) ? "R" : "W" )
	 << "," << fname << ")" << endmsg;

  if ( !io_hasitem (iocomponent) ) {
    if ( !io_register (iocomponent).isSuccess() ) {
      m_log << MSG::ERROR
	    << "could not register component [" << iocomponent->name() << "] "
	    << "with the I/O component manager !"
	    << endmsg;
      return StatusCode::FAILURE;
    }
  }

  pair<iodITR,iodITR> fit = m_cdict.equal_range(iocomponent);
  if (fit.first != fit.second) {
    for (iodITR it=fit.first; it != fit.second; ++it) {
      IoComponentEntry ioe = it->second;
      if (ioe.m_oldfname == fname) {
	if (ioe.m_iomode == iomode) {
	  m_log << MSG::INFO << "IoComponent " << ioname
	  	<< " has already had file " << fname 
	   	<< " registered with i/o mode " << iomode << endmsg;
	  return StatusCode::SUCCESS;
  } else {
	  m_log << MSG::WARNING << "IoComponent " << ioname
		<< " has already had file " << fname 
		<< " registered with a different i/o mode " << ioe.m_iomode 
		<< " - now trying " << iomode << endmsg;
  }
  }
  }
  }

  // We need to take into account that boost::filesystem::absolute() does not 
  // work for files read from eos, i.e. starting with "root:"
  std::string tmp_name = pfn.empty()?fname:pfn;
  bool from_eos = tmp_name.find("root:")==0;
  IoComponentEntry ioc(fname,(from_eos?tmp_name:boost::filesystem::absolute(tmp_name).string()),iomode);
  m_cdict.insert( pair<IIoComponent*, IoComponentEntry>(iocomponent, ioc) );

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** @brief: retrieve the new filename for a given @c IIoComponent and
 *          @param `oldfname` filename
 */
StatusCode
IoComponentMgr::io_retrieve (IIoComponent* iocomponent,
			     std::string& fname)
{
  if ( 0 == iocomponent ) {
    return StatusCode::FAILURE;
  }

  std::string ofname = fname;
  const std::string& ioname = iocomponent->name();

  m_log << MSG::DEBUG << "--> io_retrieve(" << ioname << "," << fname << ")" 
	<< endmsg;

  iodITR it;
  if (!findComp(iocomponent,ofname,it)) {
    DEBMSG << "could not find c: " << ioname << "  old_f: " << ofname << endmsg;
    return StatusCode::FAILURE;
  } else {

    IoDict_t::iterator it;
    for (it = m_cdict.equal_range(iocomponent).first;
	 it != m_cdict.equal_range(iocomponent).second;
	 ++it) {

      if (it->second.m_oldfname == ofname) {
	DEBMSG << "retrieving new name for the component " << iocomponent->name() 
	       << " old name: " << ofname 
	       << ", new name: " << it->second.m_newfname << endmsg;
	fname = it->second.m_newfname;
	return StatusCode::SUCCESS;
      }
  }
  }

  DEBMSG << "Unexpected error! Unable to find entry in the dictionary corresponding to old filename: " << ofname << endmsg; 
  return StatusCode::FAILURE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** @brief: reinitialize the I/O subsystem.
 *  This effectively calls @c IIoComponent::io_reinit on all the registered
 *  @c IIoComponent.
 */
StatusCode
IoComponentMgr::io_reinitialize ()
{
  m_log << MSG::DEBUG << "--> io_reinitialize()" << endmsg;
  m_log << MSG::DEBUG << "reinitializing I/O subsystem..." << endmsg;

  if (m_log.level() <= MSG::DEBUG) {
    m_log << MSG::DEBUG << "Listing all monitored entries: " << std::endl;
    DEBMSG << list() << endmsg;
  }

  bool allgood = true;
  for ( IoStack_t::iterator io = m_iostack.begin(), ioEnd = m_iostack.end();
	io != ioEnd;
	++io ) {
    m_log << MSG::DEBUG << " [" << (*io)->name() << "]->io_reinit()..."
	  << endmsg;
    if ( !(*io)->io_reinit().isSuccess() ) {
      allgood = false;
      m_log << MSG::ERROR << "problem in [" << (*io)->name()
	    << "]->io_reinit() !" << endmsg;
    }
    // we are done with this guy... release it
    (*io)->release();
  }

  // we are done.
  // FIXME: shall we allow for multiple io_reinitialize ?
  // m_iostack.clear();
  // m_ioregistry.clear();
  // m_cdict.clear();

  return allgood
    ? StatusCode::SUCCESS
    : StatusCode::FAILURE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
IoComponentMgr::io_update(IIoComponent* ioc, const std::string& old_fname,
			  const std::string& new_fname) {

  DEBMSG << "--> io_update(" << ioc->name() << "," 
	 << old_fname << "," << new_fname << ")" << endmsg;

  IoDict_t::iterator it;
  for (it = m_cdict.equal_range(ioc).first; 
       it != m_cdict.equal_range(ioc).second;
       ++it) {

    if (it->second.m_oldfname == old_fname) {
      DEBMSG << "updating " << ioc->name() << " f: " << old_fname << " -> "
	     << new_fname << endmsg;
      it->second.m_newfname = new_fname;
      return StatusCode::SUCCESS;
    }
  }

  return StatusCode::FAILURE;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
IoComponentMgr::io_update(IIoComponent* ioc, const std::string& work_dir) {

  DEBMSG << "--> io_update(" << ioc->name() << "," 
	 << work_dir << ")" << endmsg;

  IoDict_t::iterator it;
  for (it = m_cdict.equal_range(ioc).first; 
       it != m_cdict.equal_range(ioc).second;
       ++it) {
    
    switch(it->second.m_iomode) {
    case IIoComponentMgr::IoMode::READ:
      {
	it->second.m_newfname = it->second.m_oldabspath;
	break;
      }
    case IIoComponentMgr::IoMode::WRITE:
      {
	boost::filesystem::path oldPath(it->second.m_oldfname);
	if(oldPath.is_relative() && 
	   oldPath.filename()==oldPath.relative_path()) {

	  // Only file name was provided for writing. This is the usual mode of operation
	  // ***
	  // NB. In such cases it would make sense to set newfname=oldfname, however this will break
	  //     existing client codes, which assume that newfname contains "/" 
	  //     Thus we set newfname=workdir/oldfname
	  // ***

	  boost::filesystem::path newfname(work_dir);
	  newfname /= oldPath;
	  it->second.m_newfname = newfname.string();
	}
	else {
	  // New name should be the old absolute path
	  it->second.m_newfname = it->second.m_oldabspath;
	}

	break;
      }
    default:
      {
	// Don't know how to deal with either RW or INVALID
	m_log << MSG::ERROR << "Unable to update IoComponent for the mode " << it->second.m_iomode << endmsg;
	return StatusCode::FAILURE;
      }
    }

  } // for

  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode 
IoComponentMgr::io_update_all (const std::string& work_dir)
{
  DEBMSG << "-->io_update_all for the directory " << work_dir << endmsg;
  bool allgood = true;
  for ( IoStack_t::iterator io = m_iostack.begin(), ioEnd = m_iostack.end();
        io != ioEnd;
        ++io ) {
    if ( !io_update(*io,work_dir).isSuccess() ) {
      allgood = false;
      m_log << MSG::ERROR << "problem in [" << (*io)->name()
            << "]->io_update() !" << endmsg;
    }
  }
  return allgood ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** @brief: finalize the I/O subsystem.
 *  Hook to allow to e.g. give a chance to I/O subsystems to merge output
 *  files. Not sure how to do this correctly though...
 */
StatusCode
IoComponentMgr::io_finalize ()
{

  m_log << MSG::DEBUG << "--> io_finalize()" << endmsg;
  m_log << MSG::DEBUG << "finalizing I/O subsystem..." << endmsg;

  if (m_log.level() <= MSG::DEBUG) {
    m_log << MSG::DEBUG << "Listing all monitored entries: " << std::endl;
    DEBMSG << list() << endmsg;
  }

  bool allgood = true;
  for ( IoStack_t::iterator io = m_iostack.begin(), ioEnd = m_iostack.end();
        io != ioEnd;
        ++io ) {
    m_log << MSG::DEBUG << " [" << (*io)->name() << "]->io_finalize()..."
          << endmsg;
    if ( !(*io)->io_finalize().isSuccess() ) {
      allgood = false;
      m_log << MSG::ERROR << "problem in [" << (*io)->name()
            << "]->io_finalize() !" << endmsg;
    }
  }

  return allgood
    ? StatusCode::SUCCESS
    : StatusCode::FAILURE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool
IoComponentMgr::findComp(IIoComponent* c, const std::string& f, iodITR& itr) 
  const {

  pair<iodITR,iodITR> pit;
  if (!findComp(c,pit)) {
    itr = pit.first;
    return false;
  }
  
  for (itr = pit.first; itr != pit.second; ++itr) {
    if (itr->second.m_oldfname == f) {
      return true;
    }
  }

  return false;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool 
IoComponentMgr::findComp(IIoComponent* c, std::pair<iodITR,iodITR>& pit) const {

  pit = m_cdict.equal_range(c);

  if (pit.first == pit.second) {
    return false;
  } else {
    return true;
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool
IoComponentMgr::findComp(const std::string& c, 
			 std::pair<iodITR,iodITR>& pit) const {

  pit.first = m_cdict.end();
  pit.second = m_cdict.end();

  IoRegistry_t::const_iterator itr = m_ioregistry.find(c);
  if (itr == m_ioregistry.end()) {
    return false;
  }

  return findComp(itr->second, pit);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

std::string
IoComponentMgr::list() const {

  ostringstream ost;

  ost << "Listing all IoComponents (" << m_cdict.size() << "): " << endl;
  for (iodITR it = m_cdict.begin(); it != m_cdict.end(); ++it) {
    ost << "  " << it->first->name() << "  " << it->second
	<< endl;
  }

  return ost.str();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void 
IoComponentMgr::handle ( const Incident& i ) {

  pair<iodITR,iodITR> pit;
  

  if ( i.type() == IncidentType::BeginInputFile ) {

    const FileIncident *fi = dynamic_cast<const FileIncident*> ( &i );
    DEBMSG << "BeginInputFile:   s: " << fi->source() << "  t: " << fi->type()
	   << "  n: " << fi->fileName() << "  g: " << fi->fileGuid()
	   << endmsg;

    if (findComp(fi->source(),pit)) {
      DEBMSG << "  found component: " << endmsg;
      while (pit.first != pit.second) {
	IIoComponent* c = pit.first->first;
	IoComponentEntry e = pit.first->second;
	DEBMSG << "    c: " << c->name() << "  " << e << endmsg;
	
	++pit.first;
      }
    } else {
      DEBMSG << "  could not find component \"" << fi->source() 
	     << "\"!" << endmsg;
    }
	  


  } else if ( i.type() == IncidentType::BeginOutputFile ) {

    const FileIncident *fi = dynamic_cast<const FileIncident*> ( &i );
    DEBMSG << "BeginOutputFile:   s: " << fi->source() << "  t: " << fi->type()
	   << "  n: " << fi->fileName() << "  g: " << fi->fileGuid()
	   << endmsg;

    if (findComp(fi->source(),pit)) {
      DEBMSG << "  found component: " << endmsg;
      while (pit.first != pit.second) {
	IIoComponent* c = pit.first->first;
	IoComponentEntry e = pit.first->second;
	DEBMSG << "    c: " << c->name() << "  " << e << endmsg;
	
	++pit.first;
      }
    } else {
      DEBMSG << "  could not find component \"" << fi->source() 
	     << "\"!" << endmsg;
    }

  }

}
