///////////////////////////////////////////////////////////////////
// IFileMgr.h
// Manages all file open/reopen/close
// Author: C.Leggett 
/////////////////////////////////////////////////////////////////// 

#ifndef GAUDIKERNEL_IFILEMGR_H
#define GAUDIKERNEL_IFILEMGR_H 1

#include "GaudiKernel/IService.h"
#include "GaudiKernel/ClassID.h"

#include <map>
#include <string>
#include <vector>
#include <fcntl.h>
#include "boost/function.hpp"

namespace Io {

  //
  // Io modes
  //

  enum IoFlag {
    READ =  O_RDONLY,
    WRITE = O_WRONLY,
    RDWR =  O_RDWR,
    
    CREATE = O_CREAT,
    EXCL   = O_EXCL,
    TRUNC =  O_TRUNC,
    
    APPEND = O_APPEND,
    
    INVALID = 1<<31
  };
    
  class IoFlags {
  public:
    IoFlags():_f(INVALID){};
    IoFlags(int i):_f(i){};

    int f() const { return _f; }

    bool operator== (const IoFlags& fa) const { return ( _f == fa.f() ); }
    bool operator== (const int& fa) const { return ( _f == fa ); }

    IoFlags operator| (const IoFlags& fa) const { return (_f | fa.f()); }
    IoFlags operator| (const int& fa) const { return (_f | fa); }

    operator int() const { return _f; }

    bool isRead() const    { return (_f == READ); }
    bool isWrite() const   { return ( (_f & WRITE) != 0 ); }
    bool isRdWr() const    { return ( (_f & RDWR) != 0 ); }
    bool isInvalid() const { return ( (_f & INVALID) != 0); }

    bool match (const IoFlags& fa, bool strict=true) const {
      if ( strict ) {
	return ( _f == fa );
      }
      // only look at first 2 bits
      return (  (_f&3) == (fa&3) );
    }

    std::string bits() const {
      std::string s;
      int f(_f);
      const int SHIFT = 8 * sizeof( int ) - 1;
      const unsigned MASK = 1 << SHIFT;

      for ( int i = 1; i <= SHIFT + 1; i++ ) {
	s += ( f & MASK ? '1' : '0' );
	f <<= 1;	
	if ( i % 8 == 0 )
	    s += ' ';
      }
      return s;
    }

  private:
    int _f;
  };

  static std::string IoFlagName(IoFlags f) {
    static std::map<IoFlag, std::string> s_names;
    if (s_names.empty()) {
      s_names[READ] = "READ";
      s_names[WRITE] = "WRITE";
      s_names[RDWR] = "RDWR";
      s_names[CREATE] = "CREATE";
      s_names[EXCL] = "EXCL";
      s_names[TRUNC] = "TRUNC";
      s_names[APPEND] = "APPEND";
      s_names[INVALID] = "INVALID";
    }
    
    if ( f.isRead() ) {
      return s_names[READ];
    }
    
    std::string ff;
    for (int i=0; i<32; ++i) {
      if ( ( (1<<i) & f) != 0) {
	ff += s_names[ (IoFlag)(1<<i) ] + "|";
      }
    }
    ff.erase(ff.length()-1);
    return ff;
  }    
  

  inline IoFlags IoFlagFromName(const std::string& f) {
    static std::map<std::string, IoFlag> s_n;
    if (s_n.empty()) {
      s_n["READ"] = Io::READ;
      s_n["WRITE"] = Io::WRITE;
      s_n["RDWR"] = Io::RDWR;
      s_n["CREATE"] = Io::CREATE;
      s_n["EXCL"] = Io::EXCL;
      s_n["TRUNC"] = Io::TRUNC;
      s_n["APPEND"] = Io::APPEND;
      s_n["INVALID"] = Io::INVALID;
    }

    IoFlags fl(Io::INVALID);
    size_t j(0),k(0);      
    std::string fs;
    while ( (k=f.find("|",j)) != std::string::npos) {
      fs = f.substr(j,k-j);
      if (s_n.find(fs) == s_n.end()) {
	return Io::INVALID;
      }
      if (fl.isInvalid()) {
	fl = s_n[fs];
      } else {
	fl = fl | s_n[fs];
      }
      j = k+1;
    }
    fs = f.substr(j,f.length()-j);
    if (s_n.find(fs) == s_n.end()) {
      return Io::INVALID;
    }
    if (fl.isInvalid()) {
      fl = s_n[fs];
    } else {
      fl = fl | s_n[fs];
    }

    return fl;

  }

  inline std::ostream & operator << (std::ostream & s, const IoFlag &f) {
    return s << IoFlagName(f);
  }
  inline std::ostream& operator << (std::ostream &s, const IoFlags &f) {
    return s << IoFlagName(f);

  }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  //
  // Technologies
  //

  enum IoTech {
    UNKNOWN,
    POSIX,
    ROOT,
    BS,
    HDF5,
    SQLITE    
  };

  inline std::ostream & operator << (std::ostream & s, const IoTech &t) {
    switch (t) {
    case UNKNOWN : return s << "UNKNOWN";
    case POSIX   : return s << "POSIX";
    case ROOT    : return s << "ROOT";
    case BS      : return s << "BS";
    case HDF5    : return s << "HDF5";
    case SQLITE  : return s << "SQLITE";      
    }
    return s;
  }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  //
  // File Attributes
  //

  typedef int Fd;

  class FileAttr {
  public:

    FileAttr():m_fd(-1),m_name(""),m_desc(""),m_tech(UNKNOWN),m_flags(INVALID),
	       m_iflags(INVALID),m_fptr(0),m_isOpen(false), m_shared(false){};
    FileAttr(Fd f, const std::string& n, const std::string& d, IoTech t, 
	     IoFlags fa, void* p, bool o, bool s=false):
      m_fd(f),m_name(n),m_desc(d),m_tech(t),m_flags(fa),m_iflags(fa),m_fptr(p),
      m_isOpen(o),m_shared(s){};

    Fd          fd() const       { return m_fd; }
    const std::string& name() const   { return m_name; }
    const std::string& desc() const   { return m_desc; }
    IoTech      tech() const     { return m_tech; }
    IoFlags     flags() const    { return m_flags; }
    IoFlags     iflags() const   { return m_iflags; }
    void*       fptr() const     { return m_fptr; }
    bool        isOpen() const   { return m_isOpen; }
    bool        isShared() const { return m_shared; }


    void fd(Fd f)                   { m_fd = f; }
    void name(const std::string& n) { m_name = n; }
    void desc(const std::string& d) { m_desc = d; }
    void tech(const IoTech& t)      { m_tech = t;}
    void flags(const IoFlags& f)    { m_flags = f; }
    void iflags(const IoFlags& f)   { m_iflags = f; }
    void fptr(void* v)              { m_fptr = v; }
    void isOpen(bool b)             { m_isOpen = b; }
    void isShared(bool s)           { m_shared = s; }


    friend std::ostream& operator<< (std::ostream& os, const FileAttr& fa) {
      os << "name: \"" << fa.name() << "\"  tech: " << fa.tech() 
	 << "  desc: " << fa.desc() 
	 << "  flags: " << IoFlagName(fa.flags())
	 << "  i_flags: " << IoFlagName(fa.iflags())
	 << "  Fd: " << fa.fd() << "  ptr: " << fa.fptr()
	 << (fa.isOpen() ? "  [o]" : "  [c]" )
	 << (fa.isShared() ? " [s]" : " [u]" );
      
      return os;
    }

    bool operator== (const FileAttr& fa) const {
      return ( m_fd==fa.fd() && m_name==fa.name() && m_desc==fa.desc() &&
	       m_tech==fa.tech() && m_flags==fa.flags() && m_fptr==fa.fptr() &&
	       m_isOpen==fa.isOpen() && m_shared==fa.isShared() );
    }

    bool operator< (const FileAttr& rhs) const {
      if ( m_name != rhs.name() ) {
	return ( m_name < rhs.name() );
      } else {
	return ( m_flags < rhs.iflags() );
      }
    }

  private:

    Fd          m_fd;
    std::string m_name;
    std::string m_desc;
    IoTech      m_tech;
    IoFlags     m_flags;
    IoFlags     m_iflags;
    void*       m_fptr;
    bool        m_isOpen;
    bool        m_shared;

  };


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  //
  // Handler functions
  // 

  typedef int open_t;
  typedef int close_t;
  typedef int reopen_t;

  typedef boost::function<Io::open_t(const std::string&, Io::IoFlags, const std::string&, Io::Fd&, void*&)> bfcn_open_t;
  typedef boost::function<Io::close_t(Io::Fd)> bfcn_close_t;
  typedef boost::function<Io::close_t(void*)>  bfcn_closeP_t;
  typedef boost::function<Io::reopen_t(Io::Fd,Io::IoFlags)> bfcn_reopen_t;
  typedef boost::function<Io::reopen_t(void*,Io::IoFlags)> bfcn_reopenP_t;

  // file handler functions: open, close, reopen
  struct FileHdlr {
    IoTech tech;

    bfcn_open_t    b_open_fcn;
    bfcn_close_t   b_close_fcn;
    bfcn_closeP_t  b_closeP_fcn;
    bfcn_reopen_t  b_reopen_fcn;
    bfcn_reopenP_t b_reopenP_fcn;

    FileHdlr(): tech(UNKNOWN){};
    FileHdlr( IoTech t, bfcn_open_t o, bfcn_close_t c, bfcn_reopen_t r)
      : tech(t), b_open_fcn(o), b_close_fcn(c), b_reopen_fcn(r){};
    FileHdlr( IoTech t, bfcn_open_t o, bfcn_closeP_t c, bfcn_reopenP_t r)
      : tech(t), b_open_fcn(o), b_closeP_fcn(c), b_reopenP_fcn(r){};
    FileHdlr( IoTech t, bfcn_open_t o, bfcn_close_t c1, bfcn_closeP_t c2, 
	      bfcn_reopen_t r1,bfcn_reopenP_t r2)
      : tech(t), b_open_fcn(o), b_close_fcn(c1), b_closeP_fcn(c2), 
	b_reopen_fcn(r1), b_reopenP_fcn(r2){};

  };

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  //
  // Callback Actions
  //

  enum Action {
    OPEN = 0,
    CLOSE,
    REOPEN,
    OPEN_ERR,
    CLOSE_ERR,
    REOPEN_ERR,
    INVALID_ACTION
  };

  inline std::ostream & operator << (std::ostream & s, const Action &t) {
    switch (t) {
    case OPEN       :  return s << "OPEN"; 
    case CLOSE      :  return s << "CLOSE"; 
    case REOPEN     :  return s << "REOPEN"; 
    case OPEN_ERR   :  return s << "OPEN_ERR"; 
    case CLOSE_ERR  :  return s << "CLOSE_ERR"; 
    case REOPEN_ERR :  return s << "REOPEN_ERR"; 
    case INVALID_ACTION :  return s << "INVALID_ACTION"; 
    }
    return s;
  }

  #define FILEMGR_CALLBACK_ARGS const Io::FileAttr*, const std::string&
  typedef boost::function<StatusCode (FILEMGR_CALLBACK_ARGS) > bfcn_action_t;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

class GAUDI_API IFileMgr: virtual public IService {

 public:

  virtual ~IFileMgr();
  
  DeclareInterfaceID(IFileMgr,1,0);
  
 public:
  
  // register handler function for file technology
  virtual StatusCode regHandler(Io::FileHdlr) = 0;

  virtual StatusCode deregHandler(const Io::IoTech&) = 0;

  virtual StatusCode hasHandler(const Io::IoTech&) const = 0;

  // get handler from technology
  virtual StatusCode getHandler(const Io::IoTech&, Io::FileHdlr&) const = 0;
  // get handler from file name
  virtual StatusCode getHandler(const std::string&, Io::FileHdlr&) const = 0;

  // get file attributes from file name
  virtual int getFileAttr(const std::string&, 
			  std::vector<const Io::FileAttr*>&) const = 0;
  // get file attributes from file descriptor
  virtual StatusCode getFileAttr(const Io::Fd, const Io::FileAttr*&) const = 0;
  // get file attributes from file ptr
  virtual StatusCode getFileAttr(void*, const Io::FileAttr*&) const = 0;
  
  virtual void listHandlers() const = 0;
  virtual void listFiles() const = 0;

  // get all files known to mgr. return numbers found. 
  // will replace contents of FILES
  virtual int getFiles(std::vector<std::string> & FILES, 
		       bool onlyOpen=true) const = 0;
  virtual int getFiles(std::vector<const Io::FileAttr*> & FILES, 
		       bool onlyOpen=true) const = 0;

  // get all files of specific IoTech. returns number found. 
  // will replace contents of FILES
  virtual int getFiles(const Io::IoTech&, std::vector<std::string>& FILES, 
		       bool onlyOpen=true) const = 0;
  virtual int getFiles(const Io::IoTech&, 
		       std::vector<const Io::FileAttr*>& FILES, 
		       bool onlyOpen=true) const = 0;

  // get all file of specific IoTech and access mode. 
  // will replace contents of FILES
  // If IoTech == UNKNOWN, get all. returns number found
  virtual int getFiles(const Io::IoTech&, const Io::IoFlags& , 
		       std::vector<std::string> &FILES, 
		       bool onlyOpen=true) const =0;
  virtual int getFiles(const Io::IoTech&, const Io::IoFlags& , 
		       std::vector<const Io::FileAttr*> &FILES, 
		       bool onlyOpen=true) const =0;


  // get all descriptors known to mgr. returns number found
  virtual int getFd (std::vector<Io::Fd > & ) const = 0;
  // get all descriptors of specific IoTech. return number found
  virtual int getFd(const Io::IoTech&, std::vector<Io::Fd>&) const = 0;
  // get all descriptors of specific IoTech and access mode. 
  // If IoTech == INVALID, get all. returns number found
  virtual int getFd(const Io::IoTech&, const Io::IoFlags& , 
		    std::vector<Io::Fd> &) const = 0;
				  
  // get file name given Fd or ptr. Returns empty string if fails
  virtual const std::string& fname(const Io::Fd&) const = 0;
  virtual const std::string& fname(void*) const = 0;
  
  // get Fd given file name. Returns -1 if fails
  virtual Io::Fd fd(const std::string&) const = 0;
  virtual Io::Fd fd(void* fptr) const = 0;

  // get ptr given file name. Returns 0 if fails
  virtual void* fptr(const std::string&) const = 0;
  virtual void* fptr(const Io::Fd&) const = 0;


  virtual int getLastError(std::string&) const = 0;

 
  // Open file, get Fd and ptr
  virtual Io::open_t open(const Io::IoTech&, const std::string& caller,
                          const std::string& fname,
			  const Io::IoFlags&, Io::Fd&, void*&, 
			  const std::string& desc,
			  const bool shared=false
			  ) = 0;

  // Open file, get Fd
  virtual Io::open_t open(const Io::IoTech&, const std::string& caller,
			  const std::string& fname,
			  const Io::IoFlags&, Io::Fd&, const std::string& desc,
			  const bool shared=false
			  ) = 0;

  // Open file, get ptr
  virtual Io::open_t open(const Io::IoTech&, const std::string& caller,
			  const std::string& fname,
			  const Io::IoFlags&, void*&, const std::string& desc,
			  const bool shared=false
			  ) = 0;


  // Close file by Fd or ptr
  virtual Io::close_t  close(const Io::Fd, const std::string& caller) = 0;
  virtual Io::close_t  close(void*, const std::string& caller) = 0;

  // Reopen file by Fd or ptr
  virtual Io::reopen_t reopen(const Io::Fd, const Io::IoFlags&, 
			      const std::string& ) = 0;
  virtual Io::reopen_t reopen(void*, const Io::IoFlags&,
			      const std::string& ) = 0;


  // Callback actions
  virtual StatusCode regAction(Io::bfcn_action_t, const Io::Action&, 
			       const std::string& d="") = 0;
  virtual StatusCode regAction(Io::bfcn_action_t, const Io::Action&, 
			       const Io::IoTech&, const std::string& d="") = 0;

  // Suppress callback action(s) for specific file.
  virtual void suppressAction(const std::string&) = 0;
  virtual void suppressAction(const std::string&, const Io::Action&) = 0;


};

#endif
