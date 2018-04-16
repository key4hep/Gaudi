#define ROOTHISTCNV_ROOTDIRFCN_CPP

#include "RootDirFcn.h"
#include "GaudiKernel/Kernel.h"

#include "TDirectory.h"
#include "TFile.h"
#include <list>
#include <string>

namespace RootHistCnv
{

  //-----------------------------------------------------------------------------
  bool RootCd( const std::string& full )
  //-----------------------------------------------------------------------------
  {
    int p, i = 1;

    gDirectory->cd( "/" );
    while ( ( p = full.find( "/", i ) ) != -1 ) {
      auto sdir = full.substr( i, p - i );
      if ( !gDirectory->GetKey( sdir.c_str() ) ) {
        return false;
      }
      gDirectory->cd( sdir.c_str() );
      i = p + 1;
    }
    gDirectory->cd( full.substr( i ).c_str() );
    return true;
  }

  //-----------------------------------------------------------------------------
  bool RootMkdir( const std::string& full )
  //-----------------------------------------------------------------------------
  {

    TDirectory* gDir = gDirectory;

    int  i = 1;
    auto p = full.find( ":", 0 );
    if ( p != std::string::npos ) {
      auto fil = full.substr( 0, p );
      i        = p + 1;
      fil += ":/";
      gDirectory->cd( fil.c_str() );
    }

    std::vector<std::string> lpath;
    while ( ( p = full.find( "/", i ) ) != std::string::npos ) {
      lpath.push_back( full.substr( i, p - i ) );
      i = p + 1;
    }
    lpath.push_back( full.substr( i ) );

    if ( full.compare( 0, 1, "/" ) == 0 ) gDirectory->cd( "/" );

    for ( const auto& lp : lpath ) {
      if ( !gDirectory->GetKey( lp.c_str() ) ) {
        gDirectory->mkdir( lp.c_str() );
      }
      gDirectory->cd( lp.c_str() );
    }
    gDirectory = gDir;

    return true;
  }

  //-----------------------------------------------------------------------------
  std::string RootPwd()
  //-----------------------------------------------------------------------------
  {
    std::string dir = gDirectory->GetPath();

    return ( dir );
  }

  //-----------------------------------------------------------------------------
  bool RootTrimLeadingDir( std::string& full, std::string dir )
  //-----------------------------------------------------------------------------
  {

    if ( dir.compare( 0, 1, "/" ) != 0 ) {
      dir.insert( 0, "/" );
    }

    if ( dir.compare( dir.length() - 1, 1, "/" ) != 0 ) {
      dir += "/";
    }

    long ll = full.find( dir );
    if ( ll != 0 ) {
      return false;
    }

    full.erase( 0, dir.length() - 1 );

    return true;
  }

} // RootHistCnv namespace
