///////////////////////////////////////////////////////////////////
// POSIXFileHandler.h
// Provides open/close/reopen functions for POSIX (ASCII) files
//
// Author: C.Leggett 
/////////////////////////////////////////////////////////////////// 

#ifndef GAUDISVC_POSIXFILEHANDLER_H
#define GAUDISVC_POSIXFILEHANDLER_H 1

#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IFileMgr.h"

#include <string>

class POSIXFileHandler {

public:

  POSIXFileHandler(IMessageSvc*);

  Io::open_t openPOSIXFile(const std::string& n, const Io::IoFlags& f, 
			  const std::string& desc, Io::Fd& fd, 
			  void*& ptr);

  Io::close_t closePOSIXFile(Io::Fd fd);
  Io::reopen_t reopenPOSIXFile(Io::Fd fd, const Io::IoFlags&);

private:
  MsgStream m_log;
  int m_level;
};

#endif
