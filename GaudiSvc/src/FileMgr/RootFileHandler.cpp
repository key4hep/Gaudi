#include "GaudiKernel/IFileMgr.h"
#include "TFile.h"
#include "TROOT.h"
#include "TSSLSocket.h"
#include "TWebFile.h"

#include "GaudiKernel/MsgStream.h"
#include "RootFileHandler.h"
#include "boost/algorithm/string.hpp"

namespace ba = boost::algorithm;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

RootFileHandler::RootFileHandler( IMessageSvc* msg, const std::string& p, const std::string& c )
    : m_log( msg, "RootFileHandler" ), m_userProxy( p ), m_certDir( c )
{
  // Protect against multiple instances of TROOT
  if ( !gROOT ) {
    static TROOT root( "root", "ROOT I/O" );
  }
  m_level = msg->outputLevel( "RootFileHandler" );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

Io::open_t RootFileHandler::openRootFile( const std::string& n, const Io::IoFlags& f, const std::string& desc,
                                          Io::Fd& fd, void*& ptr )
{

  m_log.setLevel( m_level );

  if ( m_log.level() <= MSG::DEBUG )
    m_log << MSG::DEBUG << "openRootFile(\"" << n << "\"," << f << "," << desc << ")" << endmsg;

  ptr = nullptr;
  fd  = -1;

  std::string opt;

  if ( f == Io::READ ) {
    opt = "READ";
  } else if ( f == ( Io::WRITE | Io::CREATE | Io::EXCL ) ) {
    opt = "NEW";
  } else if ( f == ( Io::WRITE | Io::CREATE ) ) {
    opt = "RECREATE";
  } else if ( ( f | Io::APPEND ) != 0 ) {
    opt = "UPDATE";
  } else {
    m_log << MSG::ERROR << "Don't know how to handle IoFlag " << f << endmsg;
    return 1;
  }

  std::unique_ptr<TFile> tf;

  if ( ba::starts_with( n, "https://", ba::is_iequal{} ) || ba::starts_with( n, "http://", ba::is_iequal{} ) ) {

    if ( !f.isRead() ) {
      m_log << MSG::ERROR << "can only open web files in READ mode. "
            << "requested mode is: " << f << endmsg;
      return 1;
    }

    if ( !m_ssl_setup && ba::starts_with( n, "https://", ba::is_iequal{} ) ) {

      if ( !setupSSL() ) {
        m_log << MSG::ERROR << "Unable to setup TSSLSocket for ROOT TWebFile access over https" << endmsg;
      }
    }

    try {
      tf.reset( new TWebFile( n.c_str() ) );
    } catch ( const std::exception& Exception ) {
      m_log << MSG::ERROR << "exception caught while trying to open root"
            << " file for reading: " << Exception.what() << std::endl
            << "  -> file probably corrupt." << endmsg;
      return 1;
    } catch ( ... ) {
      m_log << MSG::ERROR << "Problems opening input file  \"" << n << "\": probably corrupt" << endmsg;
      return 1;
    }

    if ( tf && tf->IsZombie() ) {
      m_log << MSG::ERROR << "Problems opening input file  \"" << n << "\": file does not exist or in not accessible"
            << endmsg;
      tf.reset();
      return 1;
    }

  } else {

    try {
      tf.reset( TFile::Open( n.c_str(), opt.c_str() ) );
    } catch ( const std::exception& Exception ) {
      m_log << MSG::ERROR << "exception caught while trying to open root"
            << " file for reading: " << Exception.what() << std::endl
            << "  -> file probably corrupt." << endmsg;
      return 1;
    } catch ( ... ) {
      m_log << MSG::ERROR << "Problems opening input file  \"" << n << "\": probably corrupt" << endmsg;
      return 1;
    }
  }

  if ( !tf || !tf->IsOpen() ) {
    m_log << MSG::ERROR << "Unable to open ROOT file \"" << n << "\" with options \"" << opt << "\"" << endmsg;

    tf.reset();
    return 1;
  }

  fd = tf->GetFd();

  ptr = tf.release();

  if ( m_log.level() <= MSG::DEBUG ) m_log << MSG::DEBUG << "opened TFile " << ptr << " Fd: " << fd << endmsg;

  return 0;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

Io::close_t RootFileHandler::closeRootFile( void* ptr )
{

  if ( m_log.level() <= MSG::DEBUG ) m_log << MSG::DEBUG << "closeRootFile(ptr:" << ptr << ")" << endmsg;

  if ( !ptr ) {
    m_log << MSG::ERROR << "Unable to close file: ptr == 0" << endmsg;
    return -1;
  }

  TFile* tf = static_cast<TFile*>( ptr );

  try {
    tf->Close();
  } catch ( const std::exception& Exception ) {
    m_log << MSG::ERROR << "exception caught while trying to close root"
          << " file" << Exception.what() << endmsg;
    return -1;
  } catch ( ... ) {
    m_log << MSG::ERROR << "Problems closing ROOT file  \"" << tf->GetName() << "\"" << endmsg;
    return -1;
  }

  return 0;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

Io::reopen_t RootFileHandler::reopenRootFile( void*, const Io::IoFlags& )
{

  m_log << MSG::ERROR << "reopen not implemented" << endmsg;
  return -1;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

bool RootFileHandler::setupSSL()
{

  if ( m_log.level() <= MSG::DEBUG ) m_log << MSG::DEBUG << "setupSSL" << endmsg;

  // don't set anything up
  if ( m_userProxy == "NONE" || m_certDir == "NONE" ) {
    m_ssl_setup = true;
    return true;
  }

  // get stuff from $X509_USER_PROXY and $X509_CERT_DIR env vars
  if ( m_userProxy == "X509" ) {
    if ( !System::getEnv( "X509_USER_PROXY", m_userProxy ) ) {
      m_log << MSG::ERROR << "env var X509_USER_PROXY not set" << endmsg;
      return false;
    }
  }

  if ( m_certDir == "X509" ) {
    if ( !System::getEnv( "X509_CERT_DIR", m_certDir ) ) {
      m_log << MSG::ERROR << "env var X509_CERT_DIR not set" << endmsg;
      return false;
    }
  }

  if ( m_log.level() <= MSG::DEBUG )
    m_log << MSG::DEBUG << "userProxy: " << m_userProxy << "  certDir: " << m_certDir << endmsg;

  TSSLSocket::SetUpSSL( m_userProxy.c_str(), m_certDir.c_str(), m_userProxy.c_str(), m_userProxy.c_str() );

  m_ssl_setup = true;

  return true;
}
