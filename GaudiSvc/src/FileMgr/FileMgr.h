///////////////////////////////////////////////////////////////////
// FileMgr.h
// Manages all file open/reopen/close
// implementation of IFileMgr
// Author: C.Leggett 
/////////////////////////////////////////////////////////////////// 

#ifndef GAUDISVC_FILEMGR_H
#define GAUDISVC_FILEMGR_H 1

#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"

#include <string>
#include <map>
#include <list>

using Io::IoTech;
using Io::IoFlag;
using Io::IoFlags;
using Io::FileAttr;
using Io::FileHdlr;
using Io::Fd;

class RootFileHandler;
class POSIXFileHandler;

class FileMgr: public extends2<Service, IFileMgr, IIncidentListener> {
public:
  FileMgr(const std::string& name, ISvcLocator* svc);
  virtual ~FileMgr();

public:
  virtual StatusCode initialize();
  virtual StatusCode finalize();

public:
  virtual void handle(const Incident&);

public:
  virtual StatusCode regHandler(FileHdlr);
  virtual StatusCode hasHandler(const IoTech&) const;

  virtual StatusCode deregHandler(const IoTech&);

  virtual StatusCode getHandler(const IoTech&, FileHdlr&) const;
  virtual StatusCode getHandler(const std::string&, FileHdlr&) const;

  virtual int getFileAttr(const std::string&, std::vector<const FileAttr*>&) const;
  virtual StatusCode getFileAttr(const Fd, const FileAttr*&) const;
  virtual StatusCode getFileAttr(void*, const FileAttr*&) const;

  virtual void listHandlers() const;
  virtual void listFiles() const;
  virtual void listActions() const;
  virtual void listSuppression() const;

  virtual int getFiles(std::vector<std::string>&, bool onlyOpen=true) const;
  virtual int getFiles(std::vector<const Io::FileAttr*>&, 
		       bool onlyOpen=true) const;
  virtual int getFiles(const Io::IoTech&, std::vector<std::string>&,
		       bool onlyOpen=true) const;
  virtual int getFiles(const Io::IoTech&, std::vector<const Io::FileAttr*>&,
		       bool onlyOpen=true) const;
  virtual int getFiles(const Io::IoTech&, const Io::IoFlags&, 
		       std::vector<std::string>&, bool onlyOpen=true) const;
  virtual int getFiles(const Io::IoTech&, const Io::IoFlags&, 
		       std::vector<const Io::FileAttr*>&, 
		       bool onlyOpen=true) const;

  virtual int getFd(std::vector<Fd>&) const;
  virtual int getFd(const Io::IoTech&, std::vector<Io::Fd>&) const;
  virtual int getFd(const Io::IoTech&, const Io::IoFlags&, 
		    std::vector<Io::Fd> &) const;
  
  // get file name given Fd or ptr
  virtual const std::string& fname(const Io::Fd&) const;
  virtual const std::string& fname(void*) const;
  
  // get Fd or ptr given file name
  virtual Io::Fd fd(const std::string&) const;
  virtual Io::Fd fd(void*) const;

  virtual void* fptr(const std::string&) const;
  virtual void* fptr(const Io::Fd&) const;

  virtual int getLastError(std::string&) const;
  

  // Open file, get Fd and ptr
  virtual Io::open_t open(const Io::IoTech&, const std::string& caller,
			  const std::string& fname,
			  const Io::IoFlags&, Io::Fd& fd, void*& ptr,
			  const std::string& desc="", 
			  const bool shared=false);

  // Open file, get Fd
  virtual Io::open_t open(const Io::IoTech&, const std::string& caller, 
			  const std::string& fname,
			  const Io::IoFlags&, Io::Fd&, 
			  const std::string& desc="",
			  const bool shared=false);

  // Open file, get ptr
  virtual Io::open_t open(const Io::IoTech&, const std::string& caller,
			  const std::string& fname,
			  const Io::IoFlags&, void*&,
			  const std::string& desc="",
			  const bool shared=false);


  virtual Io::close_t  close(const Fd, const std::string& caller);
  virtual Io::close_t  close(void*, const std::string& caller);

  virtual Io::reopen_t reopen(const Fd, const IoFlags&, 
			      const std::string& caller);
  virtual Io::reopen_t reopen(void*, const IoFlags&, 
			      const std::string& caller);
  
  
  virtual StatusCode regAction(Io::bfcn_action_t, const Io::Action&, 
			       const std::string& desc="");
  virtual StatusCode regAction(Io::bfcn_action_t, const Io::Action&, 
			       const Io::IoTech&, const std::string& desc="");
  
  virtual void suppressAction(const std::string&);
  virtual void suppressAction(const std::string&, const Io::Action&);
  
private:
  
  StringProperty m_logfile;
  BooleanProperty m_printSummary, m_loadRootHandler, m_loadPosixHandler;
  StringProperty m_ssl_proxy, m_ssl_cert;
  
  virtual Io::open_t   open(const IoTech&, const std::string& caller,
			    const std::string& fname, const std::string& desc, 
			    const IoFlags&, Fd&, void*&,
			    const bool shared);

  typedef std::pair<Io::bfcn_action_t,std::string> bfcn_desc_t;
  typedef std::map<Io::Action, std::list<bfcn_desc_t> > actionMap;
  StatusCode execAction(Io::FileAttr*, const std::string&, const Io::Action&);
  StatusCode execActs(Io::FileAttr*, const std::string&, const Io::Action&,
		      actionMap& m);

  bool accessMatch(const Io::IoFlags&, const Io::IoFlags&, 
		   bool strict=false) const;
  bool isShareable(const std::string& filename, const Io::IoFlags&) const;


  typedef std::multimap<std::string, FileAttr*> fileMap;

  fileMap m_files;
  std::map<IoTech, FileHdlr> m_handlers;
  std::map<Fd, FileAttr*> m_descriptors;
  std::map<void*, FileAttr*> m_fptr;
  std::vector<FileAttr*> m_attr;

  typedef std::list<FileAttr*> fileList;
  fileList m_oldFiles;
  
  std::map< IoTech, actionMap > m_actions;

  typedef std::map<std::string, int> supMap;
  supMap m_supMap;

  std::string m_lastErrS;
  int m_lastErr;

  RootFileHandler* m_rfh;
  POSIXFileHandler* m_pfh;
  mutable MsgStream m_log;

};

#endif
 
