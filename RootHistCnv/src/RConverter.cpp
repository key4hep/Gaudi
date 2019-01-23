#define ROOTHISTCNV_RCONVERTER_CPP

// Include files
#include "RConverter.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"
#include "RootObjAddress.h"

#include "TDirectory.h"
#include "TFile.h"
#include "TTree.h"
#include <list>
#include <string>

namespace
{
  std::map<std::string, TFile*> s_fileMap;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::createDirectory( const std::string& loc )
//-----------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), "RConverter::createDir" );

  // Get rid of leading /NTUPLES
  std::string full = diskDirectory( loc );

  std::string fil, cur, s;
  TDirectory* gDir = gDirectory;

  TFile* tf = nullptr;
  if ( findTFile( loc, tf ).isSuccess() ) {
    tf->cd();
  }

  std::vector<std::string> lpath;
  int                      i = 1;

  auto p = full.find( ":", 0 );
  if ( p != std::string::npos ) {
    fil = full.substr( 0, p );
    i   = p + 1;
    fil += ":/";
    gDirectory->cd( fil.c_str() );
  }

  while ( ( p = full.find( "/", i ) ) != std::string::npos ) {
    s = full.substr( i, p - i );
    lpath.push_back( s );
    i = p + 1;
  }
  lpath.push_back( full.substr( i ) );

  if ( full.compare( 0, 1, "/" ) == 0 ) gDirectory->cd( "/" );

  for ( const auto& litr : lpath ) {
    cur = litr;
    if ( !gDirectory->GetKey( litr.c_str() ) ) {
      gDirectory->mkdir( litr.c_str() );
    }
    gDirectory->cd( litr.c_str() );
  }

  gDirectory = gDir;

  return StatusCode::SUCCESS;
}
//-----------------------------------------------------------------------------
std::string RootHistCnv::RConverter::diskDirectory( const std::string& loc )
//-----------------------------------------------------------------------------
{
  // Get rid of leading /NTUPLES/{INPUT_STREAM} or /stat/{INPUT_STREAM}
  std::string dir;
  long        lf1 = loc.find( "/NTUPLES/" );
  long        lf2 = loc.find( "/stat/" );
  long        ll;
  if ( lf1 != -1 ) {
    ll = loc.find( "/", lf1 + 9 );

  } else if ( lf2 != -1 ) {
    ll = loc.find( "/", lf2 + 6 );

  } else {
    MsgStream log( msgSvc(), "RConverter" );
    log << MSG::ERROR << "diskDirectory(" << loc << ")"
        << " --> no leading /NTUPLES/ or /stat/" << endmsg;
    return loc;
  }
  //  dir = loc.substr(ll+8);

  if ( ll == -1 ) {
    dir = "/";
  } else {
    dir = loc.substr( ll );
  }

  return dir;
}

//-----------------------------------------------------------------------------
std::string RootHistCnv::RConverter::directory( const std::string& loc )
//-----------------------------------------------------------------------------
{
  return diskDirectory( loc );
}

//-----------------------------------------------------------------------------
void RootHistCnv::RConverter::setDirectory( const std::string& loc )
//-----------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), "RConverter" );
  TFile*    tf = nullptr;

  std::string full = diskDirectory( loc );

  // get associated TFile
  if ( findTFile( loc, tf ).isSuccess() ) {
    tf->cd();
  } else {
    log << MSG::ERROR << "error getting TFile name " << loc << endmsg;
  }

  int         p, i = 1;
  std::string cur, sdir;

  gDirectory->cd( "/" );
  while ( ( p = full.find( "/", i ) ) != -1 ) {
    sdir = full.substr( i, p - i );
    if ( !gDirectory->GetKey( sdir.c_str() ) ) {
      log << MSG::ERROR << "cannot cd to " << full << " from " << gDirectory->GetPath() << endmsg;
      return;
    }
    gDirectory->cd( sdir.c_str() );

    i = p + 1;
  }
  gDirectory->cd( full.substr( i ).c_str() );
}

//-----------------------------------------------------------------------------
void RootHistCnv::RConverter::setDiskDirectory( const std::string& loc )
//-----------------------------------------------------------------------------
{
  setDirectory( loc );
}

//-----------------------------------------------------------------------------
std::string RootHistCnv::RConverter::getDirectory()
//-----------------------------------------------------------------------------
{
  return gDirectory->GetPath();
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::createAddress( DataObject* pObj, TDirectory* pDir, TObject* pTObj,
                                                   IOpaqueAddress*& refpAddr )
//-----------------------------------------------------------------------------
{
  // Get address again....it does not change
  IRegistry* pReg = pObj->registry();
  if ( pReg ) {
    refpAddr = pReg->address();
    if ( !refpAddr ) {
      refpAddr = new RootObjAddress( repSvcType(), objType(), pReg->name(), "", (unsigned long)( pDir ),
                                     (unsigned long)( pTObj ), pTObj );

      return StatusCode::SUCCESS;
    }
  }
  return StatusCode::FAILURE;
}

//--------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::createAddress( const std::string& rzdir, const CLID& clid, const std::string& title,
                                                   TObject* pTObj, IOpaqueAddress*& refpAddress )
//--------------------------------------------------------------------------
{
  auto pA = new RootObjAddress( repSvcType(), clid, rzdir, title, 0, 0, pTObj );

  refpAddress = pA;
  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::createAddress( const std::string& rzdir, const CLID& clid, long id, TObject* pTobj,
                                                   IOpaqueAddress*& refpAddress )
//--------------------------------------------------------------------------
{
  auto       obj    = std::to_string( id );
  StatusCode status = createAddress( rzdir, clid, obj, pTobj, refpAddress );
  if ( status.isSuccess() ) {
    unsigned long* ipar = (unsigned long*)refpAddress->ipar();
    ipar[0]             = id;
  }
  return status;
}

//-----------------------------------------------------------------------------
/// Switch to object directory (=Parent directory)
TDirectory* RootHistCnv::RConverter::changeDirectory( DataObject* pObject )
//-----------------------------------------------------------------------------
{
  if ( pObject ) {
    IRegistry* pReg = pObject->registry();
    if ( pReg ) {
      auto dataMgr = dataProvider().as<IDataManagerSvc>();
      if ( dataMgr ) {
        IRegistry* pParentReg = nullptr;
        StatusCode status     = dataMgr->objectParent( pReg, pParentReg );
        if ( status.isSuccess() ) {
          IOpaqueAddress* pParAddr = pParentReg->address();
          if ( pParAddr ) {
            TDirectory* pParentDir = (TDirectory*)pParAddr->ipar()[0];
            if ( pParentDir ) {
              gDirectory->cd( pParentDir->GetPath() );
              return pParentDir;
            }
          }
        }
      }
    }
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
/// Convert the transient object to the requested representation.
StatusCode RootHistCnv::RConverter::createRep( DataObject* pObject, IOpaqueAddress*& pAddr )
//-----------------------------------------------------------------------------
{
  GlobalDirectoryRestore restore;
  pAddr = nullptr;
  try {
    TDirectory* pParentDir = changeDirectory( pObject );
    if ( pParentDir ) {
      TObject* pTObj = createPersistent( pObject );
      if ( pTObj ) {
        pTObj->Write();
        delete pTObj;
        return createAddress( pObject, pParentDir, nullptr, pAddr );
      }
    }
  } catch ( ... ) {
  }
  MsgStream log( msgSvc(), "RConverter" );
  log << MSG::ERROR << "Failed to create persistent Object!" << endmsg;
  return StatusCode::FAILURE;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::readObject( IOpaqueAddress* /* pAddr */, DataObject*& /* refpObj */ )
{
  //    MsgStream log(msgSvc(), "RConverter::readObject");
  //    log << MSG::WARNING << pAddr->par()[0] << " <> " << pAddr->par()[1]
  //        << " <> "
  //        << pAddr->ipar()[0] << " <> " << pAddr->ipar()[1] << " <> "
  //        << pAddr->registry()->identifier() << endmsg;

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
TObject* RootHistCnv::RConverter::createPersistent( DataObject* /* pObj */ ) { return nullptr; }

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::regTFile( const std::string id, const TFile* tfile )
//-----------------------------------------------------------------------------
{
  auto imap = s_fileMap.find( id );
  if ( imap != s_fileMap.end() ) {
    MsgStream log( msgSvc(), "RConverter" );
    log << MSG::ERROR << "cannot register TTree " << id << ": already exists" << endmsg;
    return StatusCode::FAILURE;
  }
  s_fileMap[id] = const_cast<TFile*>( tfile );

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::findTFile( const std::string id, TFile*& tfile )
//-----------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), "RConverter" );
  tfile = nullptr;

  std::string idm;

  // make sure we only get first two parts of id
  int i1, i2, i3;
  i1 = id.find( "/", 0 );
  if ( i1 != 0 ) {
    log << MSG::ERROR << "Directory name does not start with \"/\": " << id << endmsg;
    return StatusCode::FAILURE;
  }
  i2 = id.find( "/", i1 + 1 );
  if ( i2 == -1 ) {
    log << MSG::ERROR << "Directory name has only one part: " << id << endmsg;
    return StatusCode::FAILURE;
  }
  i3 = id.find( "/", i2 + 1 );
  if ( i3 == -1 ) {
    idm = id;
  } else {
    idm = id.substr( 0, i3 );
  }

  auto imap = s_fileMap.find( idm );
  if ( imap == s_fileMap.end() ) return StatusCode::FAILURE;
  tfile = imap->second;
  return StatusCode::SUCCESS;
}
//-----------------------------------------------------------------------------
std::string RootHistCnv::RConverter::convertId( const std::string& id ) const
//-----------------------------------------------------------------------------
{
  bool forced = false;
  if ( id.size() > 0 && isdigit( id[0] ) ) {
    try {
      Gaudi::Property<bool> tmp;
      tmp.assign( conversionSvc().as<IProperty>()->getProperty( "ForceAlphaIds" ) );
      forced = tmp.value();
    } catch ( ... ) {
    }
  }
  if ( forced )
    return "h" + id;
  else
    return id;
}
//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::error( const std::string& msg )
//-----------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), "RootHistCnv" );
  log << MSG::ERROR << msg << endmsg;
  return StatusCode::FAILURE;
}
