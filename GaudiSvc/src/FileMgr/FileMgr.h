///////////////////////////////////////////////////////////////////
// FileMgr.h
// Manages all file open/reopen/close
// implementation of IFileMgr
// Author: C.Leggett
///////////////////////////////////////////////////////////////////

#ifndef GAUDISVC_FILEMGR_H
#define GAUDISVC_FILEMGR_H 1

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/StatusCode.h"

#include "POSIXFileHandler.h"
#include "RootFileHandler.h"

#include "boost/optional.hpp"

#include <list>
#include <map>
#include <string>

using Io::IoTech;
using Io::IoFlag;
using Io::IoFlags;
using Io::FileAttr;
using Io::FileHdlr;
using Io::Fd;

class RootFileHandler;
class POSIXFileHandler;

class FileMgr : public extends<Service, IFileMgr, IIncidentListener>
{
public:
  using extends::extends;
  ~FileMgr() override;

  StatusCode initialize() override;
  StatusCode finalize() override;

  void handle( const Incident& ) override;

  StatusCode regHandler( FileHdlr ) override;
  StatusCode hasHandler( const IoTech& ) const override;

  StatusCode deregHandler( const IoTech& ) override;

  StatusCode getHandler( const IoTech&, FileHdlr& ) const override;
  StatusCode getHandler( const std::string&, FileHdlr& ) const override;

  int        getFileAttr( const std::string&, std::vector<const FileAttr*>& ) const override;
  StatusCode getFileAttr( const Fd, const FileAttr*& ) const override;
  StatusCode getFileAttr( void*, const FileAttr*& ) const override;

  void         listHandlers() const override;
  void         listFiles() const override;
  virtual void listActions() const;     // does not override...
  virtual void listSuppression() const; // does not override...

  int getFiles( std::vector<std::string>&, bool onlyOpen = true ) const override;
  int getFiles( std::vector<const Io::FileAttr*>&, bool onlyOpen = true ) const override;
  int getFiles( const Io::IoTech&, std::vector<std::string>&, bool onlyOpen = true ) const override;
  int getFiles( const Io::IoTech&, std::vector<const Io::FileAttr*>&, bool onlyOpen = true ) const override;
  int getFiles( const Io::IoTech&, const Io::IoFlags&, std::vector<std::string>&, bool onlyOpen = true ) const override;
  int getFiles( const Io::IoTech&, const Io::IoFlags&, std::vector<const Io::FileAttr*>&,
                bool onlyOpen = true ) const override;

  int getFd( std::vector<Fd>& ) const override;
  int getFd( const Io::IoTech&, std::vector<Io::Fd>& ) const override;
  int getFd( const Io::IoTech&, const Io::IoFlags&, std::vector<Io::Fd>& ) const override;

  // get file name given Fd or ptr
  const std::string& fname( const Io::Fd& ) const override;
  const std::string& fname( void* ) const override;

  // get Fd or ptr given file name
  Io::Fd fd( const std::string& ) const override;
  Io::Fd fd( void* ) const override;

  void* fptr( const std::string& ) const override;
  void* fptr( const Io::Fd& ) const override;

  int getLastError( std::string& ) const override;

  // Open file, get Fd and ptr
  Io::open_t open( const Io::IoTech&, const std::string& caller, const std::string& fname, const Io::IoFlags&,
                   Io::Fd& fd, void*& ptr, const std::string& desc = "", const bool shared = false ) override;

  // Open file, get Fd
  Io::open_t open( const Io::IoTech&, const std::string& caller, const std::string& fname, const Io::IoFlags&, Io::Fd&,
                   const std::string& desc = "", const bool shared = false ) override;

  // Open file, get ptr
  Io::open_t open( const Io::IoTech&, const std::string& caller, const std::string& fname, const Io::IoFlags&, void*&,
                   const std::string& desc = "", const bool shared = false ) override;

  Io::close_t close( const Fd, const std::string& caller ) override;
  Io::close_t close( void*, const std::string& caller ) override;

  Io::reopen_t reopen( const Fd, const IoFlags&, const std::string& caller ) override;
  Io::reopen_t reopen( void*, const IoFlags&, const std::string& caller ) override;

  StatusCode regAction( Io::bfcn_action_t, const Io::Action&, const std::string& desc = "" ) override;
  StatusCode regAction( Io::bfcn_action_t, const Io::Action&, const Io::IoTech&,
                        const std::string& desc = "" ) override;

  void suppressAction( const std::string& ) override;
  void suppressAction( const std::string&, const Io::Action& ) override;

private:
  Gaudi::Property<std::string> m_logfile{this, "LogFile"};
  Gaudi::Property<bool>        m_printSummary{this, "PrintSummary", false};
  Gaudi::Property<bool>        m_loadRootHandler{this, "LoadROOTHandler", true};
  Gaudi::Property<bool>        m_loadPosixHandler{this, "LoadPOSIXHandler", true};

  Gaudi::Property<std::string> m_ssl_proxy{this, "TSSL_UserProxy", "X509"};
  Gaudi::Property<std::string> m_ssl_cert{this, "TSSL_CertDir", "X509"};

  virtual Io::open_t open( const IoTech&, const std::string& caller, const std::string& fname, const std::string& desc,
                           const IoFlags&, Fd&, void*&,
                           const bool shared ); // does not override...

  typedef std::pair<Io::bfcn_action_t, std::string>    bfcn_desc_t;
  typedef std::map<Io::Action, std::list<bfcn_desc_t>> actionMap;
  StatusCode execAction( Io::FileAttr*, const std::string&, const Io::Action& ) const;
  StatusCode execActs( Io::FileAttr*, const std::string&, const Io::Action&, const actionMap& m ) const;

  bool accessMatch( const Io::IoFlags&, const Io::IoFlags&, bool strict = false ) const;
  bool isShareable( const std::string& filename, const Io::IoFlags& ) const;

  typedef std::multimap<std::string, FileAttr*> fileMap;

  fileMap m_files;
  std::map<IoTech, FileHdlr> m_handlers;
  std::map<Fd, FileAttr*>    m_descriptors;
  std::vector<std::unique_ptr<FileAttr>> m_attr;

  std::vector<FileAttr*> m_oldFiles;

  std::map<IoTech, actionMap> m_actions;

  typedef std::map<std::string, int> supMap;
  supMap m_supMap;

  std::string m_lastErrS;
  int         m_lastErr;

  boost::optional<RootFileHandler>  m_rfh;
  boost::optional<POSIXFileHandler> m_pfh;
};

#endif
