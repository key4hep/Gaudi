///////////////////////////////////////////////////////////////////
// RootFileHandler.h
// Provides open/close/reopen functions for ROOT files
//
// Author: C.Leggett 
/////////////////////////////////////////////////////////////////// 

#ifndef GAUDISVC_ROOTFILEHANDLER_H
#define GAUDISVC_ROOTFILEHANDLER_H 1

#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IFileMgr.h"

#include <string>

class RootFileHandler {

public:

  RootFileHandler(IMessageSvc*, const std::string& userProxy, 
		  const std::string& certDir);

  Io::open_t openRootFile(const std::string& n, const Io::IoFlags& f, 
			  const std::string& desc, Io::Fd& fd, 
			  void*& ptr);

  Io::close_t closeRootFile(void* ptr);
  Io::reopen_t reopenRootFile(void*, const Io::IoFlags&);

  bool setupSSL();

private:
  MsgStream m_log;
  int m_level;

  std::string m_userProxy, m_certDir;  
  bool m_ssl_setup;

};

#endif
