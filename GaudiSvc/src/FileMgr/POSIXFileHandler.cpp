/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/IFileMgr.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "POSIXFileHandler.h"
#include <GaudiKernel/MsgStream.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

POSIXFileHandler::POSIXFileHandler( IMessageSvc* msg ) : m_log( msg, "POSIXFileHandler" ) {

  m_level = msg->outputLevel( "POSIXFileHandler" );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

Io::open_t POSIXFileHandler::openPOSIXFile( const std::string& n, const Io::IoFlags& f, const std::string& desc,
                                            Io::Fd& fd, void*& ptr ) {

  m_log.setLevel( m_level );

  if ( m_log.level() <= MSG::DEBUG )
    m_log << MSG::DEBUG << "openPOSIXFile(\"" << n << "\"," << f << "," << desc << ")" << endmsg;

  ptr = nullptr;
  fd  = -1;

  int mm = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

  fd       = open( n.c_str(), (int)f, mm );
  int ierr = errno;

  if ( fd == -1 ) {
    m_log << MSG::ERROR << "Error opening POSIX file \"" << n << "\": " << strerror( ierr ) << endmsg;
    return 1;
  }

  std::string m;
  if ( f.isRead() ) {
    m = "r";
  } else if ( f.isWrite() ) {
    if ( ( f & Io::APPEND ) ) {
      m = "a";
    } else {
      m = "w";
    }
  } else if ( f.isRdWr() ) {
    m = "r+";
  } else {
    m_log << MSG::ERROR << "unknown mode " << f << " when calling fdopen on " << n << endmsg;
    return 1;
  }

  if ( m_log.level() <= MSG::DEBUG ) m_log << MSG::DEBUG << "calling fdopen with mode " << m << endmsg;

  ptr  = fdopen( fd, m.c_str() );
  ierr = errno;

  if ( !ptr ) {
    m_log << MSG::ERROR << "Error calling fdopen on \"" << n << "\": " << strerror( ierr ) << endmsg;
    return 1;
  }

  m_log << MSG::DEBUG << "opened POSIX file, Fd: " << fd << "  FILE*: " << ptr << endmsg;

  return 0;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

Io::close_t POSIXFileHandler::closePOSIXFile( Io::Fd fd ) {

  if ( m_log.level() <= MSG::DEBUG ) m_log << MSG::DEBUG << "closePOSIXFile(fd:" << fd << ")" << endmsg;

  if ( fd == -1 ) {
    m_log << MSG::ERROR << "Unable to close file: FD == -1 " << endmsg;
    return -1;
  }

  if ( close( fd ) != 0 ) {
    int ierr = errno;
    m_log << MSG::ERROR << "Error closing POSIX file with FD " << fd << strerror( ierr ) << endmsg;
    return -1;
  }

  return 0;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

Io::reopen_t POSIXFileHandler::reopenPOSIXFile( Io::Fd /*fd*/, const Io::IoFlags& ) {

  m_log << MSG::ERROR << "reopen not implemented" << endmsg;
  return -1;
}
