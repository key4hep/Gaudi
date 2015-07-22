#include "FileMgr.h"

#include <fstream>

#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IJobOptionsSvc.h"

#include "RootFileHandler.h"
#include "POSIXFileHandler.h"
#include "boost/function.hpp"

#define ON_DEBUG if (UNLIKELY(outputLevel() <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(outputLevel() <= MSG::VERBOSE))

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
DECLARE_SERVICE_FACTORY(FileMgr)

using namespace std;
using namespace Io;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
namespace {

void set_bit(int& f, const unsigned int& b) {
  f |= 1 << b;
}

bool get_bit(const int& f, const unsigned int& b) {
  return f & (1 << b);
}

static const std::string s_empty = "";

constexpr struct to_name_t {
    std::string operator()(const FileAttr* f) const { return f->name(); }
    std::string operator()(const std::pair<std::string, FileAttr*>& f) const { return f.first; }
} to_name {} ;

constexpr struct select1st_t {
    template <typename T, typename S> 
    const T& operator()(const std::pair<T,S>& p) const { return p.first; }
} select1st {} ;

template <typename InputIterator, typename OutputIterator, typename UnaryOperation, typename UnaryPredicate>
OutputIterator transform_if( InputIterator first, InputIterator last,
                             OutputIterator result,
                             UnaryOperation op, 
                             UnaryPredicate pred) {
    while (first != last) {
        if (pred(*first)) *result++ = op(*first);
        ++first;
    }
    return result;
}

template <typename InputIterator, typename OutputIterator, typename UnaryOperation, typename UnaryPredicate>
OutputIterator transform_copy_if( InputIterator first, InputIterator last,
                                  OutputIterator result,
                                  UnaryOperation op, 
                                  UnaryPredicate pred) {
    while (first != last) {
        auto t = op(*first);
        if (pred(t)) *result++ = std::move(t);
        ++first;
    }
    return result;
}

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

FileMgr::FileMgr(const std::string& name, ISvcLocator* svc)
  : base_class( name, svc ), 
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
    // Where do the new-ed FileAttr get deleted? 
    // they get pushed into m_descriptors, but m_attr is presumably 
    // where they _also_ should be pushed in order to track ownership...
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::initialize() {

  // Super ugly hack to make sure we have the OutputLevel set first, so we
  // can see DEBUG printouts in update handlers.
  IJobOptionsSvc* jos(0);
  if( serviceLocator()->service( "JobOptionsSvc", jos, true ).isSuccess() ) {
    const std::vector<const Property*> *props = jos->getProperties( name() );

    if (props) {
      for ( auto& cur : *props ) {
        if ( cur->name() == "OutputLevel" ) {
            setProperty( *cur ).ignore();
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
    m_rfh.reset(  new RootFileHandler(msgSvc(), m_ssl_proxy, m_ssl_cert) );

    auto rfh = m_rfh.get(); // used in the lambdas to avoid capturing 'this'
    Io::FileHdlr hdlr(Io::ROOT,
                      [rfh](const std::string& n, const Io::IoFlags& f,
                         const std::string& desc, Io::Fd& fd,
                         void*& ptr) -> Io::open_t {
                           return rfh->openRootFile(n, f, desc, fd, ptr);
                      },
                      [rfh](void* ptr) -> Io::close_t {
                        return rfh->closeRootFile(ptr);
                      },
                      [rfh](void* ptr, const Io::IoFlags& f) -> Io::reopen_t {
                        return rfh->reopenRootFile(ptr, f);
                      });

    if (regHandler(hdlr).isFailure()) {
      m_log << MSG::ERROR
	    << "unable to register ROOT file handler with FileMgr"
	    << endmsg;
    }
  }

  if (m_loadPosixHandler.value()) {

    // setup file handler for POSIX

    msgSvc()->setOutputLevel( "POSIXFileHandler", m_outputLevel.value());
    m_pfh.reset( new POSIXFileHandler(msgSvc()) );

    auto pfh = m_pfh.get(); // used in the lambdas to avoid capturing 'this'
    Io::FileHdlr hdlp(Io::POSIX,
                      [pfh](const std::string& n, const Io::IoFlags& f,
                            const std::string& desc, Io::Fd& fd,
                            void*& ptr) -> Io::open_t {
                        return pfh->openPOSIXFile(n, f, desc, fd, ptr);
                      },
                      [pfh](Io::Fd fd) -> Io::close_t {
                        return pfh->closePOSIXFile(fd);
                      },
                      [pfh](Io::Fd fd, const Io::IoFlags& f) -> Io::reopen_t {
                         return pfh->reopenPOSIXFile(fd, f);
                      });

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
    for (const auto& itr : m_files) m_log << *(itr.second) << endl;
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
      for (const auto& itr : m_files) {
        ofs << itr.second->name() << "  " << itr.second->tech() << "  "
            << itr.second->desc() << "  " << itr.second->iflags() << endl;
      }

      set<FileAttr> fs;
      for (const auto& it2 : m_oldFiles) fs.insert(*it2);
      for (const auto& it3 : fs ) {
        ofs << it3.name() << "  " << it3.tech() << "  " << it3.desc()
            << "  " << it3.iflags()
            << ( it3.isShared() ? "  SHARED" : "" )
            << endl;
      }
      ofs.close();
    }
  }

  // cleanup FileAttrs
  m_attr.clear();

  m_rfh.reset();
  m_pfh.reset();


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

  if ( ! fh.b_open_fcn ) {
    m_log << MSG::ERROR
	  << "open handler for tech " << tech << " is NULL"
	  << endmsg;
    return StatusCode::FAILURE;
  }

  if ( ! fh.b_close_fcn && ! fh.b_closeP_fcn ) {
    m_log << MSG::ERROR
	  << "no close handler for tech " << tech << " registered"
	  << endmsg;
    return StatusCode::FAILURE;
  }

  if ( ! fh.b_reopen_fcn && ! fh.b_reopenP_fcn) {
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

  auto itr = m_handlers.find(tech);
  if (itr == m_handlers.end()) {
    m_log << MSG::ERROR << "Can't de-register tech " << tech
	  << " as it hasn't been registered!"
	  << endmsg;
    return StatusCode::FAILURE;
  } 

  m_handlers.erase(itr);
  return StatusCode::SUCCESS;

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::hasHandler( const IoTech& tech ) const {

  auto itr = m_handlers.find(tech);
  return (itr != m_handlers.end()) ? StatusCode::SUCCESS : StatusCode::FAILURE;

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
  if (getHandler(tech,fh).isFailure()) return r;


  auto fitr = m_files.equal_range(fname);

  // make sure all files with same name have same tech

  auto itr = std::find_if( fitr.first, fitr.second, [&](fileMap::const_reference i) { return i.second->tech()!=tech; } );
  if (itr != fitr.second) {
      m_log << MSG::ERROR << "when calling open on " << fname
	    << " with tech " << tech
	    << ", file already opened with different tech "
	    << itr->second->tech()
	    << endmsg;
      r = -1;
      return r;
  }


  // check for sharing

  if (shared) {

    bool shareable(true);

    for (auto itr=fitr.first; itr != fitr.second; ++itr) {
      FileAttr* fa = itr->second;

      if (! fa->isShared()) shareable = false;

      //      if ( shareable && accessMatch(fa->flags(),flags) ) 
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


  //@TODO/@FIXME: should this not be pushed into m_attr???
  // eg. m_attr.emplace_back( new FileAttr(fd,fname,desc,tech,flags,ptr,true,shared) );
  //     FileAttr* fa = m_attr.back().get(); 
  FileAttr* fa = new FileAttr(fd,fname,desc,tech,flags,ptr,true,shared);

  ON_DEBUG
    m_log << MSG::DEBUG << "opened file " << *fa << endmsg;


  if (fd == -1 && ptr == 0) {
    m_log << MSG::WARNING << "when opening " << *fa << " both File Descriptor"
	  << " and File Ptr are invalid" << endmsg;
    r = 3;
  }


  for (auto itr = fitr.first; itr != fitr.second; ++itr) {
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

  m_files.insert( { fname,fa } );

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


  auto itr = std::find_if( std::begin(m_files), std::end(m_files),
                           [&](fileMap::const_reference i) { return i.second->fd() == fd; } );


  if (itr == std::end(m_files)) {
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

  if (! fh.b_close_fcn) {
    m_log << MSG::ERROR << "no close(" << tech << ",Fd) function registered"
	  << endmsg;
    return -1;
  }

  FileAttr* fa = itr->second;

  // find how many times this file is open
  pair<fileMap::const_iterator, fileMap::const_iterator> fitr =
    m_files.equal_range(fa->name());

  int i = std::count_if(fitr.first, fitr.second, [&](fileMap::const_reference f) { 
                        return f.second->fd()==fd; } ); 

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

  auto itr = std::find_if( std::begin(m_files), std::end(m_files),
                           [&](fileMap::const_reference i ) { return i.second->fptr()==vp; } );

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
  if (! fh.b_closeP_fcn) {
    m_log << MSG::ERROR << "no close(" << tech << ",void*) function registered"
	  << endmsg;
    return -1;
  }

  FileAttr *fa = itr->second;

  // find how many times this file is open
  pair<fileMap::const_iterator, fileMap::const_iterator> fitr =
    m_files.equal_range(fa->name());

  int i = std::count_if( fitr.first, fitr.second, [&](fileMap::const_reference f)
                         { return f.second->fptr()==vp; } );

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


  auto itr = std::find_if( std::begin(m_files), std::end(m_files), [&](fileMap::const_reference f)
                           { return f.second->fd() == fd; } );

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

  if ( ! fh.b_reopen_fcn ) {
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

  auto itr = std::find_if( std::begin(m_files), std::end(m_files), 
                           [&](fileMap::const_reference f) {
                               return f.second->fptr() == vp ; } );
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

  if ( ! fh.b_reopenP_fcn ) {
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

  auto fitr = m_files.equal_range(fname);
  std::transform( fitr.first, fitr.second, std::back_inserter(fa), 
                  [](fileMap::const_reference i) { return i.second; } );

  std::copy_if( std::begin(m_oldFiles), std::end(m_oldFiles),
                std::back_inserter(fa), [&](const FileAttr* f ) { return f->name() == fname; } );

  return fa.size();

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::getFileAttr(const Fd fd, const FileAttr*& fa) const {

  auto i = std::find_if( std::begin(m_files), std::end(m_files),
                         [&](fileMap::const_reference f ) { return f.second->fd() == fd; } );
  if (i != std::end(m_files)) {
      fa = i->second;
      return StatusCode::SUCCESS;
  }

  auto j = std::find_if( std::begin(m_oldFiles), std::end(m_oldFiles),
                         [&](const FileAttr* f) { return f->fd() == fd; } );
  if (j != std::end(m_oldFiles)) {
      fa = *j;
      return StatusCode::SUCCESS;
  }

  return StatusCode::FAILURE;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::getFileAttr(void* vp, const FileAttr*& fa) const {

  auto i = std::find_if( std::begin(m_files), std::end(m_files),
                         [&](fileMap::const_reference f) { return f.second->fptr() == vp; } );
  if (i != std::end(m_files)) {
      fa = i->second;
      return StatusCode::SUCCESS;
  }

  auto j = std::find_if( std::begin(m_oldFiles), std::end(m_oldFiles),
                         [&](const FileAttr* f) { return f->fptr() == vp; } );
  if (j != std::end(m_oldFiles)) {
      fa = *j;
      return StatusCode::SUCCESS;
  }

  return StatusCode::FAILURE;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
FileMgr::getFiles(std::vector<std::string>& files, bool op) const {

  files.clear();
  auto not_in_files = [&](const std::string& i) { return std::none_of( std::begin(files), std::end(files), 
                                                                       [&](const std::string& j) { return j==i; } ); };
  transform_copy_if( std::begin(m_files), std::end(m_files), std::back_inserter(files),
                     to_name,
                     not_in_files );
  if (!op) {
    transform_copy_if( std::begin(m_oldFiles), std::end(m_oldFiles), std::back_inserter(files),
                       to_name,
                       not_in_files );
  }
  return files.size();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
FileMgr::getFiles(vector<const Io::FileAttr*>& files, bool op) const {

  files.clear();
  std::transform(std::begin(m_files), std::end(m_files), std::back_inserter(files),
                 [](fileMap::const_reference f) { return f.second; } );
  if (!op) {
    std::copy(std::begin(m_oldFiles), std::end(m_oldFiles), std::back_inserter(files));
  }
  return files.size();

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
FileMgr::getFiles(const IoTech& tech, vector<string>& files, bool op) const {

  if (tech == UNKNOWN) return getFiles(files,op);

  files.clear();
  transform_if( std::begin(m_files), std::end(m_files), std::back_inserter(files),
                to_name,
                [&](fileMap::const_reference f) { return f.second->tech() == tech && 
                                                  std::none_of( std::begin(files), std::end(files), [&](const std::string& j) { return j==f.first; } ); } );

  if (!op) {
    transform_if( std::begin(m_oldFiles), std::end(m_oldFiles), std::back_inserter(files),
                  to_name,
                  [&](const FileAttr* f) { return f->tech() == tech &&
                                                  std::none_of( std::begin(files), std::end(files), [&](const std::string& j) { return j == f->name(); } ) ; } ); 
  }
  return files.size();

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
FileMgr::getFiles(const IoTech& tech, vector<const Io::FileAttr*>& files,
                  bool op) const {

  if (tech == UNKNOWN) return getFiles(files,op);

  auto matches_tech = [&](const FileAttr* f) { return f->tech()==tech; } ;

  files.clear();
  transform_copy_if( std::begin(m_files), std::end(m_files), std::back_inserter(files),
                     [ ](fileMap::const_reference f ) { return f.second; },
                     matches_tech );
  if (!op) {
    std::copy_if( std::begin(m_oldFiles), std::end(m_oldFiles), std::back_inserter(files),
                  matches_tech );
  }

  return files.size();

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
FileMgr::getFiles(const IoTech& tech, const IoFlags& flags,
		  vector<string>& files, bool op) const {

  files.clear();

  auto not_in_files = [&](const std::string& n) { return std::none_of( std::begin(files), std::end(files),
                                                                       [&](const std::string& f) { return f==n; } ); };
  auto matches_tech_and_flags = [&](const FileAttr* f) { return ( f->tech() == tech || tech == UNKNOWN ) && f->flags() == flags ; } ;

  transform_if( std::begin(m_files), std::end(m_files), std::back_inserter(files),
                to_name,
                [&](fileMap::const_reference f) { return matches_tech_and_flags( f.second ) && not_in_files( f.first ); } );
  if (!op) {
    transform_if( std::begin(m_oldFiles), std::end(m_oldFiles), std::back_inserter(files),
                  to_name,
                  [&](const FileAttr* f) { return matches_tech_and_flags(f) && not_in_files(f->name()); } );
  }

  return files.size();

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
FileMgr::getFiles(const IoTech& tech, const IoFlags& flags,
		  vector<const Io::FileAttr*>& files, bool op) const {

  files.clear();

  auto matches_tech_and_flags = [&](const FileAttr* f) { return ( f->tech() == tech || tech == UNKNOWN ) 
                                                             && f->flags() == flags ; } ;

  transform_copy_if( std::begin(m_files), std::end(m_files), std::back_inserter(files),
                     [](fileMap::const_reference f) { return f.second; },
                     matches_tech_and_flags );
  if (!op) {
    std::copy_if( std::begin(m_oldFiles), std::end(m_oldFiles), std::back_inserter(files),
                  matches_tech_and_flags );
  }

  return files.size();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// get all descriptors known
// return number found

int
FileMgr::getFd(vector<Fd>& fd) const {

  std::transform( std::begin(m_descriptors), std::end(m_descriptors), 
                  std::back_inserter(fd), select1st );
  return m_descriptors.size();

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// get all descriptors given tech
// return number found

int
FileMgr::getFd(const IoTech& tech, vector<Fd>& fd) const {

  if (tech == UNKNOWN) return getFd( fd );

  fd.clear();
  transform_if( std::begin(m_descriptors), std::end(m_descriptors), std::back_inserter(fd),
                select1st,
                [&](const std::pair<Fd,FileAttr*>& d) { return d.second->tech()==tech; } );

  return fd.size();

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// get all descriptors of given tech and flags
// return number found

int
FileMgr::getFd(const IoTech& tech, const IoFlags& flags, vector<Fd>& fd) const {

  fd.clear();
  transform_if( m_descriptors.begin(), m_descriptors.end(),
                std::back_inserter(fd),
                select1st,
                [&](const std::pair<Fd,FileAttr*>& d) { 
                      return (d.second->tech() == tech || tech == UNKNOWN) &&
                             ( d.second->flags() == flags ); 
  } );
  return fd.size();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

const std::string&
FileMgr::fname(const Io::Fd& fd) const {

  auto itr = std::find_if( std::begin(m_files), std::end(m_files), 
                           [&](fileMap::const_reference f) { return f.second->fd() == fd; } );
  return (itr!=std::end(m_files)) ? itr->second->name() : s_empty; 

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

const std::string&
FileMgr::fname(void* vp) const {

  auto itr = std::find_if( m_files.begin(), m_files.end(), [&](fileMap::const_reference f) {
    return  f.second->fptr() == vp;
  });
  return itr!=m_files.end() ? itr->second->name() : s_empty;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

Io::Fd
FileMgr::fd(const std::string& fname) const {

  auto fitr = m_files.equal_range(fname);
  auto itr = std::find_if( fitr.first, fitr.second, [](fileMap::const_reference f) {
      return f.second->fd() != -1;
  } );
  return itr!=fitr.second ? itr->second->fd() : -1 ;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

Io::Fd
FileMgr::fd(void* fptr) const {

  auto itr = std::find_if(m_files.begin(),m_files.end(),[&](fileMap::const_reference f) {
      return f.second->fptr() == fptr;
  } );
  return itr!=m_files.end() ? itr->second->fd() : -1 ;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void*
FileMgr::fptr(const std::string& fname) const {
  auto fitr = m_files.equal_range(fname);
  auto itr = std::find_if( fitr.first, fitr.second, [](fileMap::const_reference f) -> bool {
                return f.second->fptr();
  } );
  return itr!=fitr.second ? itr->second->fptr() : nullptr;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void*
FileMgr::fptr(const Io::Fd& fd) const {

  auto itr = std::find_if(m_files.begin(),m_files.end(),[&](fileMap::const_reference f) {
        return f.second->fd() == fd;
  } );
  return itr!=m_files.end() ?  itr->second->fptr() : nullptr;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
FileMgr::listFiles() const {

  m_log << MSG::INFO << "listing registered files ["
	<< (m_files.size() + m_oldFiles.size() )
	<< "]:" << endl;

  for (auto& itr : m_files    ) m_log << *(itr.second) << endl;
  for (auto& it2 : m_oldFiles ) m_log << *it2 << endl;

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

  auto itr = m_handlers.find(tech);
  if (itr == m_handlers.end()) {
    m_log << MSG::ERROR
	  << "no handler for tech " << tech << " registered"
	  << endmsg;
    return StatusCode::FAILURE;
  }
  hdlr = itr->second;
  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::getHandler(const std::string& fname, Io::FileHdlr& hdlr) const {

  auto fitr = m_files.equal_range(fname);

  if (fitr.first == fitr.second) {
    m_log << MSG::ERROR
	  << "no file \"" << fname << "\" registered. Cannot determine tech"
	  << endmsg;
    return StatusCode::FAILURE;
  }

  auto itr = fitr.first;
  IoTech tech = itr->second->tech();

  ++itr;
  while( itr != fitr.second ) {
    if ( itr->second->tech() != tech ) {
      m_log << MSG::ERROR
	    << "multiple technologies registered for file \"" << fname
	    << "\". Cannot determine handler" << endmsg;
      return StatusCode::FAILURE;
    }
    ++itr;
  }

  return getHandler(tech,hdlr);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
FileMgr::listHandlers() const {

  m_log << MSG::INFO
	<< "Listing registered handlers:" << endl;

  for (const auto& itr : m_handlers ) m_log << "    " << itr.first << endl;
  m_log << endmsg;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::regAction(bfcn_action_t bf, const Io::Action& a, const std::string& d) {

  ON_DEBUG
    m_log << MSG::DEBUG << "registering " << a << " action "
	  << System::typeinfoName(bf.target_type()) << endmsg;

  if (d.empty()) {
    m_actions[Io::UNKNOWN][a].emplace_back(bf,System::typeinfoName(bf.target_type()));
  } else {
    m_actions[Io::UNKNOWN][a].emplace_back(bf,d);
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

  if (d.empty()) {
    m_actions[t][a].emplace_back(bf, System::typeinfoName(bf.target_type()));
  } else {
    m_actions[t][a].emplace_back(bf,d);
  }

  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
FileMgr::listActions() const {

  m_log << MSG::INFO << "listing registered actions" << endl;

  for (const auto& iit : m_actions) {
    Io::IoTech t = iit.first;
    const actionMap& m = iit.second;

    if (m.size() != 0) {
      m_log << " --- Tech: ";
      if (t == Io::UNKNOWN) {
        m_log << "ALL ---" << endl;
      } else {
        m_log << t << " ---" << endl;
      }
      for (const auto& iia : m ) {
        if (iia.second.size() != 0) {
            for (const auto& it2 : iia.second ) {
                m_log << "   " << iia.first << "  "
		              << it2.second << endl;
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
		      const Io::Action& a) const {

  Io::IoTech tech = fa->tech();

  StatusCode s1,s2;

  auto itr = m_actions.find(Io::UNKNOWN);

  if (itr != m_actions.end() && itr->second.size() != 0) {
    const actionMap &m = itr->second;
    s1 = execActs(fa, caller, a, m);
  }

  itr = m_actions.find(tech);
  if (itr != m_actions.end() && itr->second.size() != 0) {
    const actionMap &m = itr->second;
    s2 = execActs(fa, caller, a, m);
  }

  return (s1.isFailure() || s2.isFailure()) ? StatusCode::FAILURE 
                                            : StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
FileMgr::execActs(Io::FileAttr* fa, const std::string& caller,
		    const Io::Action& a, const actionMap& m) const {

  auto mitr = m.find(a);

  if (mitr == m.end() || mitr->second.size() == 0) {
    return StatusCode::SUCCESS;
  }

  ON_DEBUG
    m_log << MSG::DEBUG
	  << "executing " << mitr->second.size()  << " " << a
	  << " actions on "
	  << *fa << " from "
	  << caller
	  << endmsg;


  bool fail(false);

  auto it2 = m_supMap.find(fa->name());
  if (it2 != m_supMap.end()) {
    if (get_bit(it2->second,a) || get_bit(it2->second,Io::INVALID_ACTION)) {
      ON_DEBUG
	m_log << MSG::DEBUG << "     --> suppressing callback action for "
	      << a
	      << endmsg;
      return StatusCode::SUCCESS;
    }
  }

  for (const auto& itr : mitr->second ) {

    ON_DEBUG
      m_log << MSG::DEBUG << "executing "
	    << itr.second << endmsg;

    if ( (((itr.first))(fa,caller)).isFailure() ) {
      m_log << MSG::WARNING << "execution of "
	    << itr.second << " on " << *fa
	    << " failed during " << a << " action"
	    << endmsg;
      fail = true;
    }

  }

  return  fail ? StatusCode::FAILURE : StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool
FileMgr::accessMatch(const Io::IoFlags& fold, const Io::IoFlags& fnew,
		     bool /*strict*/) const {

    ON_VERBOSE
      m_log << MSG::VERBOSE << "accessMatch  old: " << fold
	    << "  new: " << fnew
	    << endmsg;

  return  ( ((fold == Io::READ)  &&  (fnew == Io::READ)) ||
          ( (fold & Io::WRITE) != 0  &&  (fnew & Io::WRITE) != 0) ||
          ( (fold & Io::RDWR)  != 0  &&  (fnew & Io::RDWR)  != 0) ) ;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
FileMgr::suppressAction(const std::string& f) {

  return suppressAction(f,Io::INVALID_ACTION);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
FileMgr::suppressAction(const std::string& f, const Io::Action& a) {

  auto it2 = m_supMap.find(f);
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

  for (auto it2=m_supMap.begin(); it2 != m_supMap.end(); ++it2) {
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
