/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
///////////////////////////////////////////////////////////////////
// POSIXFileHandler.h
// Provides open/close/reopen functions for POSIX (ASCII) files
//
// Author: C.Leggett
///////////////////////////////////////////////////////////////////

#pragma once

#include <GaudiKernel/IFileMgr.h>
#include <GaudiKernel/IMessageSvc.h>
#include <GaudiKernel/MsgStream.h>

#include <string>

class POSIXFileHandler {

public:
  POSIXFileHandler( IMessageSvc* );

  Io::open_t openPOSIXFile( const std::string& n, const Io::IoFlags& f, const std::string& desc, Io::Fd& fd,
                            void*& ptr );

  Io::close_t  closePOSIXFile( Io::Fd fd );
  Io::reopen_t reopenPOSIXFile( Io::Fd fd, const Io::IoFlags& );

private:
  MsgStream m_log;
  int       m_level;
};
