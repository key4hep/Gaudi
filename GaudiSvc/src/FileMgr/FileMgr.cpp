#include "FileMgr.h"

#include <fstream>

#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IJobOptionsSvc.h"

#include "RootFileHandler.h"
#include "POSIXFileHandler.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

#define ON_DEBUG if (UNLIKELY(outputLevel() <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(outputLevel() <= MSG::VERBOSE))

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
DECLARE_SERVICE_FACTORY(FileMgr)

using namespace std;
using namespace Io;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void set_bit(int& f, const unsigned int& b) {
  f |= 1 << b;
}

bool get_bit(const int& f, const unsigned int& b) {
  return f & (1 << b);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

FileMgr::FileMgr(const std::string& name, ISvcLocator* svc)
  : base_class( name, svc ),m_rfh(0), m_pfh(0),
    m_log(msgSvc(), name )
 {

   declareProperty("LogFile",m_logfile="");
   declareProperty("PrintSummary",m_printSummary=false);
   declareProperty("LoadROOTHandler", m_loadRootHandler=true);
   declareProperty("LoadPOSIXHandler", m_loadPosixHandler=true);
   
   declareProperty("TSSL_UserProxy", m_ssl_proxy="X509");
   declareProperty("TSSL_CertDir", m_ssl_cert="X509");

   m_lastErrS = "";

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

FileMgr::~FileMgr() {

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::initialize() {

  // Super ugly hack to make sure we have the OutputLevel set first, so we
  // can see DEBUG printouts in update handlers.
  IJobOptionsSvc* jos(0);
  if( serviceLocator()->service( "JobOptionsSvc", jos, true ).isSuccess() ) {
    const std::vector<const Property*> *props = jos->getProperties( name() );

    if (props != NULL) {
      for ( std::vector<const Property*>::const_iterator cur = props->begin();
	    cur != props->end(); cur++) {
	if ( (*cur)->name() == "OutputLevel" ) {
	  setProperty( **cur ).ignore();
	  m_log.setLevel( m_outputLevel.value() );
	  break;
	}
      }
    }
  }

  StatusCode status = Service::initialize();
  m_log.setLevel( m_outputLevel.value() );

  if (status.isFailure()) {

    ON_DEBUG
      m_log << MSG::DEBUG << "Failed to initialize the base class (Service)"
	    << endmsg;
    return status;
  }

  ON_VERBOSE
    m_log << MSG::VERBOSE << "Initializing FileMgr" << endmsg;

  if (m_loadRootHandler.value()) {

    // setup file handler for ROOT
    
    msgSvc()->setOutputLevel( "RootFileHandler", m_outputLevel.value());
    m_rfh = new RootFileHandler(msgSvc(), m_ssl_proxy, m_ssl_cert);
    
    Io::FileHdlr hdlr(Io::ROOT, 
		      boost::bind( &RootFileHandler::openRootFile, m_rfh, _1,_2,_3,_4,_5),
		      (Io::bfcn_closeP_t) boost::bind( &RootFileHandler::closeRootFile, m_rfh, _1),
		      (Io::bfcn_reopenP_t) boost::bind( &RootFileHandler::reopenRootFile, m_rfh, _1,_2) );
    
    if (regHandler(hdlr).isFailure()) {
      m_log << MSG::ERROR
	    << "unable to register ROOT file handler with FileMgr"
	    << endmsg;
    }
  }

  if (m_loadPosixHandler.value()) {

    // setup file handler for POSIX
    
    msgSvc()->setOutputLevel( "POSIXFileHandler", m_outputLevel.value());
    m_pfh = new POSIXFileHandler(msgSvc());
    
    Io::FileHdlr hdlp(Io::POSIX, 
		      boost::bind( &POSIXFileHandler::openPOSIXFile, m_pfh, _1,_2,_3,_4,_5),
		      (Io::bfcn_close_t) boost::bind( &POSIXFileHandler::closePOSIXFile, m_pfh, _1),
		      (Io::bfcn_reopen_t) boost::bind( &POSIXFileHandler::reopenPOSIXFile, m_pfh, _1,_2) );
    
    if (regHandler(hdlp).isFailure()) {
      m_log << MSG::ERROR
	    << "unable to register ROOT file handler with FileMgr"
	    << endmsg;
    }
  }



  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::finalize() {
  ON_VERBOSE
    m_log << MSG::VERBOSE << "FileMgr::finalize()" << endmsg;


  if (m_printSummary || outputLevel() <= MSG::DEBUG) {
    listHandlers();
    listFiles();
    listActions();
    listSuppression();
  }


  if (m_files.size() > 0) {
    m_log << MSG::WARNING 
	  << "At finalize, the following files remained open:"
	  << endl;
    map<string,FileAttr*>::const_iterator itr;
    for (itr=m_files.begin(); itr != m_files.end(); ++itr) {
      m_log << *(itr->second) << endl;	
    }
    m_log << endmsg;
  }



  if (m_logfile.value() != "") {
    std::ofstream ofs;
    ofs.open(m_logfile.value().c_str());
    if (!ofs) {
      m_log << MSG::ERROR << "Unable to open output file \"" << m_logfile.value() 
	    << "\" for writing"
	    << endmsg;
    } else {
      ON_DEBUG
	m_log << MSG::DEBUG << "Saving log to \"" << m_logfile.value() << "\"" 
	      << endmsg;
      fileMap::const_iterator itr;
      for (itr=m_files.begin(); itr != m_files.end(); ++itr) {
	ofs << itr->second->name() << "  " << itr->second->tech() << "  "
	    << itr->second->desc() << "  " << itr->second->iflags() << endl;
      }

      set<FileAttr> fs;
      set<FileAttr>::const_iterator it3;
      list<FileAttr*>::const_iterator it2;
      for (it2=m_oldFiles.begin(); it2 != m_oldFiles.end(); ++it2) {
	fs.insert(**it2);
      }

      for (it3=fs.begin(); it3!=fs.end(); ++it3) {
	ofs << (it3)->name() << "  " << (it3)->tech() << "  " << (it3)->desc()
	    << "  " << (it3)->iflags()
	    << ( (it3->isShared()) ? "  SHARED" : "" )
	    << endl;
      }

      ofs.close();
    }
  }

  // cleanup FileAttrs
  for (vector<FileAttr*>::iterator itr = m_attr.begin(); itr != m_attr.end();
	 ++itr) {
    delete(*itr);
  }
  m_attr.clear();

  delete m_rfh;
  delete m_pfh;


  StatusCode status = Service::finalize();

  ON_DEBUG 
  if ( status.isSuccess() )
      m_log << MSG::DEBUG << "Service finalised successfully" << endmsg;

  return status;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
FileMgr::handle(const Incident& /*inc*/) {

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::regHandler(FileHdlr fh) {

  IoTech tech = fh.tech;

  if (m_handlers.find(tech) != m_handlers.end()) {
    m_log << MSG::WARNING
	  << "Handler for IoTech " << tech << " already registered. Ignoring."
	  << endmsg;
    return StatusCode::SUCCESS;
  }

  if (fh.b_open_fcn.empty()) {
    m_log << MSG::ERROR
	  << "open handler for tech " << tech << " is NULL"
	  << endmsg;
    return StatusCode::FAILURE;
  }

  if (fh.b_close_fcn.empty() && fh.b_closeP_fcn.empty()) {
    m_log << MSG::ERROR
	  << "no close handler for tech " << tech << " registered"
	  << endmsg;
    return StatusCode::FAILURE;
  }

  if (fh.b_reopen_fcn.empty() && fh.b_reopenP_fcn.empty()) {
    m_log << MSG::ERROR
	  << "no reopen handler for tech " << tech << " registered"
	  << endmsg;
    return StatusCode::FAILURE;
  }


  ON_DEBUG 
    m_log << MSG::DEBUG
	  << "Successfully registered handler for tech \"" << tech << "\""
	  << endmsg;

  m_handlers[tech] = fh;

  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::deregHandler( const IoTech& tech ) {
  FileHdlr hdlr;

  map<IoTech,FileHdlr>::iterator itr = m_handlers.find(tech);
  if (itr == m_handlers.end()) {
    m_log << MSG::ERROR << "Can't de-register tech " << tech 
	  << " as it hasn't been registered!"
	  << endmsg;
    return StatusCode::FAILURE;
  } else {
    m_handlers.erase(itr);
  }

  return StatusCode::SUCCESS;

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::hasHandler( const IoTech& tech ) const {

  map<IoTech,FileHdlr>::const_iterator itr = m_handlers.find(tech);
  if (itr != m_handlers.end()) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

open_t
FileMgr::open( const IoTech& tech, const std::string& caller,
	       const std::string& fname,
	       const IoFlags& flags, Fd& fd, void*& ptr,
	       const std::string& desc, bool sh) {

  return open(tech, caller, fname, desc, flags, fd, ptr, sh);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
open_t
FileMgr::open( const IoTech& tech, const std::string& caller,
	       const std::string& fname,
	       const IoFlags& flags, Fd& fd, const std::string& desc,
	       bool sh) {

  void* dummy(0);
  return open(tech, caller, fname, desc, flags, fd, dummy, sh);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
open_t
FileMgr::open( const IoTech& tech, const std::string& caller,
	       const std::string& fname,
	       const IoFlags& flags, void*& ptr, const std::string& desc,
	       bool sh) {

  Fd dummy(-1);
  return open(tech, caller, fname, desc, flags, dummy, ptr, sh);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

open_t
FileMgr::open( const IoTech& tech, const std::string& caller, 
	       const std::string& fname, 
	       const std::string& desc,
	       const IoFlags& flags, Fd& fd, void*& ptr, bool shared) {

  // return codes: ok == 0, warning > 0, error < 0
  //   0: ok
  // 
  // WARNING:
  //   1: file already open with existing FileAttributes
  //   2: file already open with different FileAttributes
  //   3: file opened, but fd and ptr are both invalid
  // 
  // ERRORS:
  //  -1: no handler for TECH
  //  -2: file already open with different tech
  //  -3: file asked to be opened in shared mode, but other open file
  //      exist that are marked unshared
  //  -4: error calling tech specific open function
  

  ON_VERBOSE
    m_log << MSG::VERBOSE << "open(" << tech << ","
	  << caller
	  << ",\"" << fname << "\",\"" 
	  << desc << "\","
	  << flags
	  << ( shared ? ",shared" : ",unshared")
	  << ")"
	  << endmsg;

  open_t r = -1;
  FileHdlr fh;

  if (getHandler(tech,fh).isFailure()) {
    return r;
  }


  pair<fileMap::const_iterator,fileMap::const_iterator> fitr =
    m_files.equal_range(fname);
  fileMap::const_iterator itr;


  // make sure all files with same name have same tech
  
  for (itr=fitr.first; itr != fitr.second; ++itr) {
    if (itr->second->tech() != tech) {

      m_log << MSG::ERROR << "when calling open on " << fname 
	    << " with tech " << tech 
	    << ", file already opened with different tech "
	    << itr->second->tech()
	    << endmsg;

      r = -1;

      return r;
    }
  }


  // check for sharing

  if (shared) {

    bool shareable(true);
    
    for (itr=fitr.first; itr != fitr.second; ++itr) {
      FileAttr* fa = itr->second;

      if (! fa->isShared()) {
        shareable = false;
      }

      //      if ( shareable && accessMatch(fa->flags(),flags) ) {
      if ( shareable && fa->flags().match(flags,false) ) {

	ON_DEBUG
	  m_log << MSG::DEBUG << " found shared file: "
		<< *fa << endmsg;

	fd = fa->fd();
	ptr = fa->fptr();
	r = 0;
      }
    }
    
    if (!shareable) {
      // at least one of the other files was not marked shared.
      fd = -1;
      ptr = 0;
      r = -1;
    }
  }

    
  if (r != 0) {

    try {
      r = fh.b_open_fcn(fname,flags,desc,fd,ptr);
    } catch (const boost::bad_function_call& err) {
      m_log << MSG::ERROR << "when calling open handler for " << tech 
	    << " on file "
	    << fname << " caught " << err.what() << endmsg;
      return -4;
    } catch (...) {
      m_log << MSG::ERROR << "when calling open handler for " << tech 
	    << " on fle "
	    << fname << " caught an unknown exception." << endmsg;
    return -4;
    }
    
    
    if (r != 0) {
      m_log << MSG::WARNING
	    << "open of file \"" << fname << "\", tech: \"" << tech 
	    << "\", flags: \"" << flags << "\" requested by "
	    << caller
	    << " failed. return code: " << r
	    << endmsg;
      
      FileAttr xfa(-1,fname,desc,tech,flags,0,false);
      execAction( &xfa, caller, Io::OPEN_ERR ).ignore();
      
      return r;
    }
  }
  

  FileAttr *fa = new FileAttr(fd,fname,desc,tech,flags,ptr,true,shared);

  ON_DEBUG
    m_log << MSG::DEBUG << "opened file " << *fa << endmsg;

  
  if (fd == -1 && ptr == 0) {
    m_log << MSG::WARNING << "when opening " << *fa << " both File Descriptor"
	  << " and File Ptr are invalid" << endmsg;
    r = 3;
  }


  for (itr = fitr.first; itr != fitr.second; ++itr) {
    if (fa->flags() == Io::READ || shared) {
      // ok
    } else if (*fa == *(itr->second) ) {
      m_log << MSG::WARNING << "open call for file \"" << fname
	    << "\" returned a pre-existing file with identical"
	    << " FileAttributes: " << *fa << endmsg;
      r = 1;
    } else {
      m_log << MSG::WARNING << "open call for file \"" << fname
	    << "\" returned a pre-existing file with different"
	    << " FileAttributes -"
	    << endl << "old: " << *(itr->second)
	    << endl << "new: " << *fa << endmsg;
      r = 2;
    }
  }

  m_files.insert( pair<string,FileAttr*>(fname,fa) );
  
  // execute all our open callbacks
  if (execAction( fa, caller, Io::OPEN ).isFailure()) {
    m_log << MSG::WARNING
	  << "at least one open callback action failed"
	  << endmsg;
  }


  return r;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

close_t
FileMgr::close( Fd fd, const std::string& caller ) {

  // return codes:
  //   < 0 : error condition
  //     0 : actual close of one file
  //   > 0 : shared file, removed from list, no actual close, returns
  //         number of shared files still open.

  ON_VERBOSE
    m_log << MSG::VERBOSE << "close("  << fd << ")" 
	  << endmsg;

  close_t r = -1;


  fileMap::iterator itr;
  for (itr = m_files.begin(); itr != m_files.end(); ++itr) {
    if (itr->second->fd() == fd) {      
      break;
    }
  }

  if (itr == m_files.end()) {
    m_log << MSG::ERROR << "unknown file descriptor \"" << fd 
	  << "\" when calling close()"
	  << endmsg;
    return r;
  }
  

  IoTech tech = itr->second->tech();

  FileHdlr fh;

  if (getHandler(tech,fh).isFailure()) {
    return r;
  }

  if (fh.b_close_fcn.empty()) {
    m_log << MSG::ERROR << "no close(" << tech << ",Fd) function registered"
	  << endmsg;
    return -1;
  }

  FileAttr* fa = itr->second;

  // find how many times this file is open
  pair<fileMap::const_iterator, fileMap::const_iterator> fitr =
    m_files.equal_range(fa->name());

  int i(0);
  for (fileMap::const_iterator it=fitr.first; it != fitr.second; ++it) {
    if (it->second->fd() == fd) {
      i++;
    }
  }

  ON_VERBOSE
    m_log << MSG::VERBOSE << "   ref count: "  << i
	  << endmsg;


  if (i > 1 && fa->isShared()) {
    // file open multiple times. don't do the actual close
    ON_DEBUG
      m_log << MSG::DEBUG << "closing file " << fa->name() << " opened "
	    << i << " times with Fd " << fd << endmsg;
    m_files.erase(itr);

    r = i-1;

  } else if (i == 1 || (i>1 && !fa->isShared()) ) {
    ON_DEBUG
      m_log << "closing " << *fa << endmsg;

  
    try {
      r = fh.b_close_fcn(fd);
    } catch (const boost::bad_function_call& err) {
      m_log << MSG::ERROR << "when calling close handler for " << tech 
	    << " on file descriptor " 
	    << fd << " caught " << err.what() << endmsg;
      execAction(fa, caller, Io::CLOSE_ERR ).ignore();
      return -1;
    } catch (...) {
      m_log << MSG::ERROR << "when calling close handler for " << tech 
	    << " on file descriptor "
	    << fd << " caught an unknown exception." << endmsg;
      execAction(fa, caller, Io::CLOSE_ERR ).ignore();
      return -1;
    }

    if (r < 0) {
      m_log << MSG::WARNING
	    << "close of file with FD \"" << fd
	    << "\", name: \"" << fa->name()
	    << "\", tech: \"" << tech << "\" failed"
	    << endmsg;
      
      execAction(fa, caller, Io::CLOSE_ERR ).ignore();
      
      return r;
    }

    m_files.erase(itr);

  } else if (i <= 0) {
    // this should never happen!
    m_log << MSG::ERROR
	  << "ref count < 0 when closing " << fa 
	  << ". This should never happen" 
	  << endmsg;
    return -1;

  }

  fa->fd(-1);
  fa->flags(INVALID);
  fa->isOpen(false);
  fa->fptr(0);
  m_oldFiles.push_back( fa );

    
  // exec all callbacks
  if (execAction(fa, caller, Io::CLOSE).isFailure()) {
    m_log << MSG::WARNING
	  << "at least one close callback action failed"
	  << endmsg;
  }
    

  return r;

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

close_t
FileMgr::close(void* vp, const std::string& caller) {
  
  // return codes:
  //   < 0 : error condition
  //     0 : actual close of one file
  //   > 0 : shared file, removed from list, no actual close, returns
  //         number of shared files still open.

  ON_VERBOSE
    m_log << MSG::VERBOSE << "close(" << vp << ")" 
	  << endmsg;
  
  close_t r = -1;
  
  fileMap::iterator itr;
  for (itr = m_files.begin(); itr != m_files.end(); ++itr) {
    if (itr->second->fptr() == vp) {      
      break;
    }
  }

  if (itr == m_files.end()) {
    m_log << MSG::ERROR << "unknown file ptr \"" << vp 
	  << "\" when calling close()"
	  << endmsg;
    return r;
  }

  IoTech tech = itr->second->tech();

  FileHdlr fh;

  if (getHandler(tech,fh).isFailure()) {
    return r;
  }
  if (fh.b_closeP_fcn.empty()) {
    m_log << MSG::ERROR << "no close(" << tech << ",void*) function registered"
	  << endmsg;
    return -1;
  }

  FileAttr *fa = itr->second;

  // find how many times this file is open
  pair<fileMap::const_iterator, fileMap::const_iterator> fitr =
    m_files.equal_range(fa->name());

  int i(0);
  for (fileMap::const_iterator it=fitr.first; it != fitr.second; ++it) {
    if (it->second->fptr() == vp) {
      i++;
    }
  }

  ON_VERBOSE
    m_log << MSG::VERBOSE << "   ref count: "  << i
	  << endmsg;

  if (i > 1 && fa->isShared()) {  
    // file open multiple times in shared access. don't do the actual close
    ON_DEBUG
      m_log << MSG::DEBUG << "closing file " << fa->name() << " opened "
	    << i << " times with fptr " << vp << endmsg;
    m_files.erase(itr);

    r = i-1;

  } else if (i == 1 || (i>1 && !fa->isShared()) ) {
    ON_DEBUG
      m_log << MSG::DEBUG << "closing: " << *fa << endmsg;
    
    try {
      r = fh.b_closeP_fcn(vp);
    } catch (const boost::bad_function_call& err) {
      m_log << MSG::ERROR << "when calling close handler for " << tech 
	    << " on file " << fa->name() 
	    << " caught " << err.what() << endmsg;
      execAction(fa, caller, CLOSE_ERR ).ignore();
      return -1;
    } catch (...) {
      m_log << MSG::ERROR << "when calling close handler for " << tech 
	    << " on file " << fa->name()
	    << " caught an unknown exception." << endmsg;
      execAction(fa, caller, CLOSE_ERR ).ignore();
      return -1;
    }
    
    if (r < 0) {
      m_log << MSG::WARNING
	    << "close of file with ptr \"" << vp
	    << "\", name: \"" << fa->name()
	    << "\", tech: \"" << tech << "\" failed"
	    << endmsg;
      
      return r;
    }

    m_files.erase(itr);

  } else {
    // this should never happen!
    m_log << MSG::ERROR
	  << "ref count: " << i << " < 0 when closing " << fa 
	  << ". This should never happen" 
	  << endmsg;
    return -1;

  }
    
  fa->fd(-1);
  fa->flags(INVALID);
  fa->isOpen(false);
  fa->fptr(0);
  m_oldFiles.push_back( fa );
    

  // exec all callbacks
  if (execAction(fa, caller, CLOSE).isFailure()) {
    m_log << MSG::WARNING
	  << "at least one close callback action failed"
	  << endmsg;
  }

  return r;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

reopen_t
FileMgr::reopen(Fd fd, const IoFlags& flags, const std::string& caller) {
  
  ON_VERBOSE
    m_log << MSG::VERBOSE << "reopen("  << fd << "," << flags
	  << "," << caller << ")" 
	  << endmsg;

  reopen_t r = -1;


  fileMap::iterator itr;
  for (itr = m_files.begin(); itr != m_files.end(); ++itr) {
    if (itr->second->fd() == fd) {      
      break;
    }
  }

  if (itr == m_files.end()) {
    m_log << MSG::ERROR << "unregistered FD \"" << fd 
	  << "\" when calling reopen()"
	  << endmsg;
    return r;
  }

  FileAttr *fa = itr->second;
  IoTech tech = fa->tech();

  FileHdlr fh;
  
  if (getHandler(tech,fh).isFailure()) {
    return r;
  }
  
  fa->flags( flags );
  
  if (fh.b_reopen_fcn.empty()) {
    m_log << MSG::ERROR << "no reopen(" << tech << ",Fd) function registered"
	  << endmsg;
    return -1;
  }

// FIXME: what does it mean to call reopen on a shared file?

  
  try {
    r = fh.b_reopen_fcn(fd,flags);
  } catch (const boost::bad_function_call& err) {
    m_log << MSG::ERROR << "when calling reopen handler for " << tech 
	  << " on file descriptor " << fd << " with flags " 
	  << flags
	  << " caught " << err.what() << endmsg;
    return -1;
  } catch (...) {
    m_log << MSG::ERROR << "when calling reopen handler for " << tech 
	  << " on file descriptor " << fd << " with flags "
	  << flags
	  << " caught an unknown exception." << endmsg;
    return -1;
  }
  
  if (r < 0) {
    m_log << MSG::WARNING
	  << "reopen of file with FD \"" << fd
	  << "\", name: \"" << fa->name()
	  << "\", tech: \"" << tech 
	  << "\", flags: \"" << flags << "\" failed"
	  << endmsg;
    
    execAction(fa, caller, Io::REOPEN_ERR ).ignore();
    
    return r;
    
  }
  
  fa->isOpen(true);
  fa->flags(flags);
  
  // exec all callbacks
  if (execAction(fa, caller, Io::REOPEN).isFailure()) {
    m_log << MSG::WARNING
	  << "at least one reopen callback action failed"
	  << endmsg;
  }

  return r;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

reopen_t
FileMgr::reopen(void* vp, const IoFlags& flags, const std::string& caller) {
  ON_VERBOSE
    m_log << MSG::VERBOSE << "reopen("  << vp << "," << flags
	  << "," << caller << ")" 
	  << endmsg;

  reopen_t r = -1;

  fileMap::iterator itr;
  for (itr = m_files.begin(); itr != m_files.end(); ++itr) {
    if (itr->second->fptr() == vp) {      
      break;
    }
  }

  if (itr == m_files.end()) {
    m_log << MSG::ERROR
	  << "unregistered file ptr \"" << vp 
	  << "\" when calling reopen()"
	  << endmsg;
    return r;
  }

  FileAttr *fa = itr->second;
  FileHdlr fh;
  IoTech tech = fa->tech();

  if (getHandler(tech,fh).isFailure()) {
    return r;
  }

  if (fh.b_reopenP_fcn.empty()) {
    m_log << MSG::ERROR << "no reopen(" << tech << ",void*) function registered"
	  << endmsg;
    return -1;
  }

  try {
    r = fh.b_reopenP_fcn(vp,flags);
  } catch (const boost::bad_function_call& err) {
    m_log << MSG::ERROR << "when calling reopen handler for " << tech 
	  << " on file " << fa->name() << " with flags " 
	  << flags
	  << " caught " << err.what() << endmsg;
    return -1;
  } catch (...) {
    m_log << MSG::ERROR << "when calling reopen handler for " << tech 
	  << " on file " << fa->name() << " with flags " 
	  << flags
	  << " caught an unknown exception." << endmsg;
    return -1;
  }

  if (r < 0) {
    m_log << MSG::WARNING
	  << "reopen of file with ptr \"" << vp
	  << "\", name: \"" << fa->name()
	  << "\", tech: \"" << tech 
	  << "\", flags: \"" << flags << "\" failed"
	  << endmsg;

    execAction(fa, caller, Io::REOPEN_ERR ).ignore();

    return r;

  }

  fa->isOpen(true);
  fa->flags(flags);

  // exec all callbacks
  if (execAction(fa, caller, Io::REOPEN).isFailure()) {
    m_log << MSG::WARNING
	  << "at least one reopen callback action failed"
	  << endmsg;
  }

  return r;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
FileMgr::getFileAttr(const std::string& fname, vector<const FileAttr*>& fa) const {

  fa.clear();
  pair<fileMap::const_iterator,fileMap::const_iterator> fitr =
    m_files.equal_range(fname);

  fileMap::const_iterator itr;
  for (itr=fitr.first; itr != fitr.second; ++itr) {
    fa.push_back( (itr->second) );
  }

  fileList::const_iterator it2;
  for (it2=m_oldFiles.begin(); it2!=m_oldFiles.end(); ++it2) {
    if ( (*it2)->name() == fname ) {
      fa.push_back( *it2 );
    }
  }

  return fa.size();


}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::getFileAttr(const Fd fd, const FileAttr*& fa) const {

  fileMap::const_iterator itr;
  for (itr = m_files.begin(); itr != m_files.end(); ++itr) {
    if (itr->second->fd() == fd) {
      fa = (itr->second);
      return StatusCode::SUCCESS;
    }
  }

  fileList::const_iterator it2;
  for (it2=m_oldFiles.begin(); it2!=m_oldFiles.end(); ++it2) {
    if ( (*it2)->fd() == fd ) {
      fa = *it2;
      return StatusCode::SUCCESS;
    }
  }

  return StatusCode::FAILURE;
  
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
StatusCode
FileMgr::getFileAttr(void* vp, const FileAttr*& fa) const {

  fileMap::const_iterator itr;
  for (itr = m_files.begin(); itr != m_files.end(); ++itr) {
    if (itr->second->fptr() == vp) {
      fa = (itr->second);
      return StatusCode::SUCCESS;
    }
  }

  fileList::const_iterator it2;
  for (it2=m_oldFiles.begin(); it2!=m_oldFiles.end(); ++it2) {
    if ( (*it2)->fptr() == vp ) {
      fa = *it2;
      return StatusCode::SUCCESS;
    }
  }

  return StatusCode::FAILURE;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
FileMgr::getFiles(vector<string>& files, bool op) const {

  files.clear();

  map<string,FileAttr*>::const_iterator itr;
  for (itr=m_files.begin(); itr != m_files.end(); ++itr) {
    if (find( files.begin(), files.end(), itr->first) == files.end()) {
      files.push_back(itr->first);
    }
  }

  if (!op) {
    std::list<FileAttr*>::const_iterator it2;
    for (it2=m_oldFiles.begin(); it2!=m_oldFiles.end(); ++it2) {
      if (find(files.begin(), files.end(), (*it2)->name()) == files.end()) {
	files.push_back( (*it2)->name());
      }
    }
  }

  return files.size();

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
FileMgr::getFiles(vector<const Io::FileAttr*>& files, bool op) const {

  files.clear();

  map<string,FileAttr*>::const_iterator itr;
  for (itr=m_files.begin(); itr != m_files.end(); ++itr) {
    files.push_back( (itr->second) );
  }

  if (!op) {
    std::list<FileAttr*>::const_iterator it2;
    for (it2=m_oldFiles.begin(); it2!=m_oldFiles.end(); ++it2) {
	files.push_back( *it2 );
    }
  }

  return files.size();

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
FileMgr::getFiles(const IoTech& tech, vector<string>& files, bool op) const {

  if (tech == UNKNOWN) {
    return getFiles(files,op);
  }

  files.clear();

  map<string,FileAttr*>::const_iterator itr;
  for (itr=m_files.begin(); itr != m_files.end(); ++itr) {
    if (itr->second->tech() == tech && 
	find(files.begin(),files.end(),itr->first) == files.end()) {
      files.push_back(itr->first);
    }
  }

  if (!op) {
    std::list<FileAttr*>::const_iterator it2;
    for (it2=m_oldFiles.begin(); it2!=m_oldFiles.end(); ++it2) {
      if ( (*it2)->tech() == tech && 
	  find(files.begin(), files.end(), (*it2)->name()) == files.end()) {
	files.push_back( (*it2)->name());
      }
    }
  }

  return files.size();

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
FileMgr::getFiles(const IoTech& tech, vector<const Io::FileAttr*>& files, 
		  bool op) const {

  if (tech == UNKNOWN) {
    return getFiles(files,op);
  }

  files.clear();

  map<string,FileAttr*>::const_iterator itr;
  for (itr=m_files.begin(); itr != m_files.end(); ++itr) {
    if (itr->second->tech() == tech) {
      files.push_back( (itr->second) );
    }
  }

  if (!op) {
    std::list<FileAttr*>::const_iterator it2;
    for (it2=m_oldFiles.begin(); it2!=m_oldFiles.end(); ++it2) {
      if ( (*it2)->tech() == tech) {
	files.push_back( *it2 );
      }
    }
  }

  return files.size();

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
FileMgr::getFiles(const IoTech& tech, const IoFlags& flags, 
		  vector<string>& files, bool op) const {

  files.clear();

  map<string,FileAttr*>::const_iterator itr;
  for (itr=m_files.begin(); itr != m_files.end(); ++itr) {
    if (itr->second->tech() == tech || tech == UNKNOWN) {
      if ( itr->second->flags() == flags ) {
	if (find( files.begin(), files.end(), itr->first ) == files.end()) {
	  files.push_back(itr->first);
	}
      }
    }
  }

  if (!op) {
    std::list<FileAttr*>::const_iterator it2;
    for (it2=m_oldFiles.begin(); it2!=m_oldFiles.end(); ++it2) {
      if ( (*it2)->tech() == tech || tech == UNKNOWN) {
	if ( (*it2)->flags() == flags ) {
	  if (find(files.begin(), files.end(), (*it2)->name()) == files.end()){
	    files.push_back( (*it2)->name());
	  }
	}
      }
    }
  }

  return files.size();

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
FileMgr::getFiles(const IoTech& tech, const IoFlags& flags, 
		  vector<const Io::FileAttr*>& files, bool op) const {

  files.clear();

  map<string,FileAttr*>::const_iterator itr;
  for (itr=m_files.begin(); itr != m_files.end(); ++itr) {
    if (itr->second->tech() == tech || tech == UNKNOWN) {
      if ( itr->second->flags() == flags ) {
	files.push_back( (itr->second) );
      }
    }
  }

  if (!op) {
    std::list<FileAttr*>::const_iterator it2;
    for (it2=m_oldFiles.begin(); it2!=m_oldFiles.end(); ++it2) {
      if ( (*it2)->tech() == tech || tech == UNKNOWN) {
	if ( (*it2)->flags() == flags ) {
	    files.push_back( *it2);
	}
      }
    }
  }

  return files.size();

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// get all descriptors known
// return number found

int
FileMgr::getFd(vector<Fd>& fd) const {

  map<Fd,FileAttr*>::const_iterator itr;
  for (itr=m_descriptors.begin(); itr != m_descriptors.end(); ++itr) {
    fd.push_back(itr->first);
  }

  return m_descriptors.size();

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// get all descriptors given tech
// return number found

int
FileMgr::getFd(const IoTech& tech, vector<Fd>& fd) const {

  if (tech == UNKNOWN) {
    return getFd( fd );
  }

  fd.clear();
  map<Fd,FileAttr*>::const_iterator itr;
  for (itr=m_descriptors.begin(); itr != m_descriptors.end(); ++itr) {
    if (itr->second->tech() == tech) {
      fd.push_back(itr->first);
    }
  }

  return fd.size();

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// get all descriptors of given tech and flags
// return number found

int
FileMgr::getFd(const IoTech& tech, const IoFlags& flags, vector<Fd>& fd) const {

  fd.clear();
  map<Fd,FileAttr*>::const_iterator itr;
  for (itr=m_descriptors.begin(); itr != m_descriptors.end(); ++itr) {
    if (itr->second->tech() == tech || tech == UNKNOWN) {
      if ( itr->second->flags() == flags ) {
	fd.push_back(itr->first);
      }
    }
  }

  return fd.size();

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

const std::string&
FileMgr::fname(const Io::Fd& fd) const {

  fileMap::const_iterator itr;
  for (itr = m_files.begin(); itr != m_files.end(); ++itr) {
    if (itr->second->fd() == fd) {      
      return itr->second->name();
    }
  }

  static const std::string s_empty = "";
  return s_empty;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

const std::string&
FileMgr::fname(void* vp) const {

  fileMap::const_iterator itr;
  for (itr = m_files.begin(); itr != m_files.end(); ++itr) {
    if (itr->second->fptr() == vp) {      
      return itr->second->name();
    }
  }

  static const std::string s_empty = "";
  return s_empty;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

Io::Fd
FileMgr::fd(const std::string& fname) const {

  pair<fileMap::const_iterator,fileMap::const_iterator> fitr =
    m_files.equal_range(fname);

  fileMap::const_iterator itr;
  for (itr=fitr.first; itr != fitr.second; ++itr) {
    if (itr->second->fd() != -1) {
      return itr->second->fd();
    }
  }

  return -1;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

Io::Fd
FileMgr::fd(void* fptr) const {

  fileMap::const_iterator itr;
  for (itr = m_files.begin(); itr != m_files.end(); ++itr) {
    if (itr->second->fptr() == fptr) {
      return itr->second->fd();
    }
  }

  return -1;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void*
FileMgr::fptr(const std::string& fname) const {
  pair<fileMap::const_iterator,fileMap::const_iterator> fitr =
    m_files.equal_range(fname);

  fileMap::const_iterator itr;
  for (itr=fitr.first; itr != fitr.second; ++itr) {
    if (itr->second->fptr() != 0) {
      return itr->second->fptr();
    }
  }

  return 0;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void*
FileMgr::fptr(const Io::Fd& fd) const {

  fileMap::const_iterator itr;
  for (itr = m_files.begin(); itr != m_files.end(); ++itr) {
    if (itr->second->fd() == fd) {
      return itr->second->fptr();
    }
  }

  return 0;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
FileMgr::listFiles() const {

  m_log << MSG::INFO << "listing registered files [" 
	<< (m_files.size() + m_oldFiles.size() )
	<< "]:" << endl;

  fileMap::const_iterator itr;
  for (itr=m_files.begin(); itr != m_files.end(); ++itr) {
    m_log << *(itr->second) << endl;
  }

  for (list<FileAttr*>::const_iterator it2=m_oldFiles.begin(); 
       it2 != m_oldFiles.end(); ++it2) {
    m_log << **it2 << endl;
  }

  m_log << endmsg;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
FileMgr::getLastError(std::string& err) const {

  err = m_lastErrS;

  return m_lastErr;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::getHandler(const Io::IoTech& tech, Io::FileHdlr& hdlr) const {

  std::map<IoTech,FileHdlr>::const_iterator itr = m_handlers.find(tech);
  if (itr != m_handlers.end()) {
    hdlr = itr->second;
    return StatusCode::SUCCESS;
  } else {
    m_log << MSG::ERROR
	  << "no handler for tech " << tech << " registered"
	  << endmsg;
    return StatusCode::FAILURE;
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::getHandler(const std::string& fname, Io::FileHdlr& hdlr) const {

  pair<fileMap::const_iterator,fileMap::const_iterator> fitr =
    m_files.equal_range(fname);
  
  if (fitr.first == fitr.second) {
    m_log << MSG::ERROR
	  << "no file \"" << fname << "\" registered. Cannot determine tech"
	  << endmsg;
    return StatusCode::FAILURE;
  }

  fileMap::const_iterator itr = fitr.first;
  IoTech tech = itr->second->tech();

  itr++;
  while( itr != fitr.second ) {
    if ( itr->second->tech() != tech ) {
      m_log << MSG::ERROR
	    << "multiple technologies registered for file \"" << fname
	    << "\". Cannot determine handler" << endmsg;
      return StatusCode::FAILURE;
    }
    itr++;
  }
    
  return getHandler(tech,hdlr);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
FileMgr::listHandlers() const {
  
  m_log << MSG::INFO
	<< "Listing registered handlers:" << endl;

  map<IoTech,FileHdlr>::const_iterator itr;
  for (itr=m_handlers.begin(); itr != m_handlers.end(); ++itr) {
    
    m_log << "    " << itr->first << endl;
  }
  m_log << endmsg;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::regAction(bfcn_action_t bf, const Io::Action& a, const std::string& d) {

  ON_DEBUG 
    m_log << MSG::DEBUG << "registering " << a << " action " 
	  << System::typeinfoName(bf.target_type()) << endmsg;

  if (d == "") {
    m_actions[Io::UNKNOWN][a].push_back
      (make_pair(bf,System::typeinfoName(bf.target_type())));
  } else {
    m_actions[Io::UNKNOWN][a].push_back(make_pair(bf,d));
  }


  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::regAction(bfcn_action_t bf, const Io::Action& a, const Io::IoTech& t,
		   const std::string& d) {

  ON_DEBUG 
    m_log << MSG::DEBUG << "registering " << a << " action " 
	  << System::typeinfoName(bf.target_type()) 
	  << " for tech " << t << endmsg;

  if (d == "") {
    m_actions[t][a].push_back
      (make_pair(bf, System::typeinfoName(bf.target_type())));
  } else {
    m_actions[t][a].push_back(make_pair(bf,d));
  }

  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
FileMgr::listActions() const {

  m_log << MSG::INFO << "listing registered actions" << endl;

  actionMap::const_iterator itr;

  typedef StatusCode (*ptf)(const Io::FileAttr&, const std::string&);

  map<Io::IoTech, actionMap>::const_iterator iit;
  for (iit = m_actions.begin(); iit != m_actions.end(); ++iit) {
    Io::IoTech t = iit->first;
    const actionMap& m = iit->second;

    if (m.size() != 0) {
      m_log << " --- Tech: ";
      if (t == Io::UNKNOWN) {
	m_log << "ALL ---" << endl;
      } else {
	m_log << t << " ---" << endl;
      }

      actionMap::const_iterator iia;
      for (iia = m.begin() ; iia != m.end(); ++iia) {
	if (iia->second.size() != 0) {
	  // for (list<bfcn_action_t>::const_iterator it2 = iia->second.begin();
	  //      it2 != iia->second.end(); ++it2) {
	  for (list<bfcn_desc_t>::const_iterator it2 = iia->second.begin();
	       it2 != iia->second.end(); ++it2) {
	    
	    m_log << "   " << iia->first << "  " 
		  << it2->second
		  << endl;
	  }
	}
      }

    }
  }


  m_log << endmsg;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::execAction( Io::FileAttr* fa, const std::string& caller,
		      const Io::Action& a) {

  Io::IoTech tech = fa->tech();

  StatusCode s1,s2;

  if (m_actions[Io::UNKNOWN].size() != 0) {
    actionMap &m = m_actions[Io::UNKNOWN];
    s1 = execActs(fa, caller, a, m);
  }

  if (m_actions[tech].size() != 0) {
    actionMap &m = m_actions[tech];
    s2 = execActs(fa, caller, a, m);
  }

  if (s1.isFailure() || s2.isFailure()) {
    return StatusCode::FAILURE;
  } else {
    return StatusCode::SUCCESS;
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode 
FileMgr::execActs(Io::FileAttr* fa, const std::string& caller, 
		    const Io::Action& a, actionMap& m) {

  if (m[a].size() == 0) {
    return StatusCode::SUCCESS;
  }

  ON_DEBUG
    m_log << MSG::DEBUG 
	  << "executing " << m[a].size() << " " << a
	  << " actions on " 
	  << *fa << " from " 
	  << caller
	  << endmsg;


  bool fail(false);

  supMap::const_iterator it2 = m_supMap.find(fa->name());
  if (it2 != m_supMap.end()) {
    if (get_bit(it2->second,a) || get_bit(it2->second,Io::INVALID_ACTION)) {
      ON_DEBUG
	m_log << MSG::DEBUG << "     --> suppressing callback action for " 
	      << a
	      << endmsg;
      return StatusCode::SUCCESS;
    }
  }

  for (list<bfcn_desc_t>::iterator itr = m[a].begin(); 
       itr != m[a].end(); ++itr) {

    ON_DEBUG
      m_log << MSG::DEBUG << "executing " 
	    << itr->second << endmsg;

    if ( (((itr->first))(fa,caller)).isFailure() ) { 
      m_log << MSG::WARNING << "execution of "
	    << itr->second << " on " << *fa
	    << " failed during " << a << " action"
	    << endmsg;
      fail = true; 
    }

  }

  if (fail) {
    return StatusCode::FAILURE;
  } else {
    return StatusCode::SUCCESS;
  }  

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool
FileMgr::accessMatch(const Io::IoFlags& fold, const Io::IoFlags& fnew,
		     bool /*strict*/) const {

    ON_VERBOSE
      m_log << MSG::VERBOSE << "accessMatch  old: " << fold
	    << "  new: " << fnew
	    << endmsg;


  if ( ((fold == Io::READ)  &&  (fnew == Io::READ)) ||
       ((fold & Io::WRITE) != 0  &&  (fnew & Io::WRITE) != 0) ||
       ((fold & Io::RDWR)  != 0  &&  (fnew & Io::RDWR)  != 0) ) {
    return true;
  } else {
    return false;
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
FileMgr::suppressAction(const std::string& f) {

  return suppressAction(f,Io::INVALID_ACTION);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
FileMgr::suppressAction(const std::string& f, const Io::Action& a) {
  
  supMap::iterator it2 = m_supMap.find(f);
  if (it2 == m_supMap.end()) {
    int b(0);
    set_bit(b,a);
    m_supMap[f] = b;
  } else {
    set_bit(it2->second, a);
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
FileMgr::listSuppression() const {

  if (m_supMap.size() == 0)  return;

  m_log << MSG::INFO << "listing suppressed file actions" << endl;

  supMap::const_iterator it2;
  for (it2=m_supMap.begin(); it2 != m_supMap.end(); ++it2) {
    m_log << "  " << it2->first;
    if (get_bit(it2->second, Io::INVALID_ACTION)) {
      m_log << " ALL" << endl;
    } else {
      for (int i=0; i<Io::INVALID_ACTION; ++i) {
	if (get_bit(it2->second,i)) { m_log << " " << (Io::Action)i; }
      }
      m_log << endl;
    }
  }

  m_log << endmsg;

}
