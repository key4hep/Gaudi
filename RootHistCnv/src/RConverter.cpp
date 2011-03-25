// $Id: RConverter.cpp,v 1.13 2007/01/08 17:16:02 mato Exp $
#define ROOTHISTCNV_RCONVERTER_CPP

// Include files
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/xtoa.h"
#include "RConverter.h"
#include "RootObjAddress.h"

#include "TDirectory.h"
#include "TFile.h"
#include "TTree.h"
#include <string>
#include <list>

namespace  {
  std::map<std::string,TFile*> s_fileMap;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::createDirectory(const std::string& loc)
//-----------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), "RConverter::createDir");

  // Get rid of leading /NTUPLES
  std::string full;
  full = diskDirectory( loc );

  int p,i;
  std::string fil,cur,s;
  TDirectory *gDir;

  gDir = gDirectory;

  TFile *tf;
  if ( findTFile(loc,tf).isSuccess() ) {
    tf->cd();
  }

  std::list<std::string> lpath;
  i = 1;

  if ( (p=full.find(":",0)) != -1 ) {
    fil = full.substr(0,p);
    i = p+1;
    fil += ":/";
    gDirectory->cd(fil.c_str());
  }

  while ( (p = full.find("/",i)) != -1) {
    s = full.substr(i,p-i);
    lpath.push_back(s);
    i = p+1;
  }
  lpath.push_back( full.substr(i,full.length()-i) );

  if ( full.substr(0,1) == "/") {
    gDirectory->cd("/");
  }

  std::list<std::string>::const_iterator litr;
  for(litr=lpath.begin(); litr!=lpath.end(); ++litr) {
    cur = *litr;
    if (! gDirectory->GetKey(litr->c_str()) ) {
      gDirectory->mkdir(litr->c_str());
    }
    gDirectory->cd(litr->c_str());
  }

  gDirectory = gDir;

  return StatusCode::SUCCESS;
}
//-----------------------------------------------------------------------------
std::string RootHistCnv::RConverter::diskDirectory(const std::string& loc)
//-----------------------------------------------------------------------------
{
  // Get rid of leading /NTUPLES/{INPUT_STREAM} or /stat/{INPUT_STREAM}
  std::string dir;
  long lf1 = loc.find("/NTUPLES/");
  long lf2 = loc.find("/stat/");
  long ll;
  if (lf1 != -1) {
    ll = loc.find("/",lf1+9);

  } else if (lf2 != -1) {
    ll = loc.find("/",lf2+6);

  } else {
    MsgStream log(msgSvc(), "RConverter");
    log << MSG::ERROR << "diskDirectory(" << loc << ")"
	<< " --> no leading /NTUPLES/ or /stat/" << endmsg;
    return loc;
  }
  //  dir = loc.substr(ll+8,loc.length()-ll-8);

  if (ll == -1) {
    dir = "/";
  } else {
    dir = loc.substr(ll,loc.length()-ll);
  }

  return dir;
}

//-----------------------------------------------------------------------------
std::string RootHistCnv::RConverter::directory(const std::string& loc)
//-----------------------------------------------------------------------------
{
  return ( diskDirectory(loc) );
}

//-----------------------------------------------------------------------------
void RootHistCnv::RConverter::setDirectory(const std::string& loc)
//-----------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), "RConverter");
  std::string full, id;
  TFile *tf;

  full = diskDirectory( loc );

  // get associated TFile
  if ( findTFile(loc,tf).isSuccess() ) {
    tf->cd();
  } else {
    log << MSG::ERROR << "error getting TFile name " << loc << endmsg;
  }

  int p,i=1;
  std::string cur,sdir;

  gDirectory->cd("/");
  while ( (p = full.find("/",i)) != -1) {
    sdir = full.substr(i,p-i);
    if (! gDirectory->GetKey(sdir.c_str()) ) {
      log << MSG::ERROR << "cannot cd to " << full << " from "
	  << gDirectory->GetPath() << endmsg;
      return;
    }
    gDirectory->cd(sdir.c_str());

    i = p+1;
  }
  gDirectory->cd( full.substr(i,full.length()-i).c_str() );
}

//-----------------------------------------------------------------------------
void RootHistCnv::RConverter::setDiskDirectory(const std::string& loc)
//-----------------------------------------------------------------------------
{
  setDirectory(loc);
}

//-----------------------------------------------------------------------------
std::string RootHistCnv::RConverter::getDirectory()
//-----------------------------------------------------------------------------
{
  std::string dir = gDirectory->GetPath();
  return (dir);
}


//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::createAddress(DataObject* pObj,
                                                  TDirectory* pDir,
                                                  TObject* pTObj,
                                                  IOpaqueAddress*& refpAddr)
//-----------------------------------------------------------------------------
{
  // Get address again....it does not change
  IRegistry* pReg = pObj->registry();
  if ( 0 != pReg )    {
    refpAddr = pReg->address();
    if ( 0 == refpAddr )    {
      refpAddr = new RootObjAddress(repSvcType(),
				    objType(),
				    pReg->name(),
				    "",
				    (unsigned long)(pDir),
 				    (unsigned long)(pTObj),
				    pTObj);

      return StatusCode::SUCCESS;
    }
  }
  return StatusCode::FAILURE;
}

//--------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::createAddress(const std::string& rzdir,
						  const CLID& clid,
						  const std::string& title,
						  TObject* pTObj,
						  IOpaqueAddress*& refpAddress)
//--------------------------------------------------------------------------
{
  RootObjAddress* pA = new RootObjAddress(repSvcType(),
					  clid,
					  rzdir,
					  title,
					  0,
 					  0,
					  pTObj );

  refpAddress = pA;
  return StatusCode::SUCCESS;
}


//--------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::createAddress(const std::string& rzdir,
						  const CLID& clid,
						  long id,
						  TObject* pTobj,
						  IOpaqueAddress*& refpAddress)
//--------------------------------------------------------------------------
{
  std::ostringstream obj; obj << id;
  StatusCode status = createAddress(rzdir, clid, obj.str(), pTobj, refpAddress);
  if ( status.isSuccess() )   {
    unsigned long* ipar = (unsigned long*)refpAddress->ipar();
    ipar[0] = id;
  }
  return status;
}

//-----------------------------------------------------------------------------
/// Switch to object directory (=Parent directory)
TDirectory* RootHistCnv::RConverter::changeDirectory(DataObject* pObject)
//-----------------------------------------------------------------------------
{
  if ( pObject )    {
    IRegistry* pReg = pObject->registry();
    if ( pReg )    {
      SmartIF<IDataManagerSvc> dataMgr(dataProvider());
      if ( dataMgr.isValid() )    {
        IRegistry* pParentReg = 0;
        StatusCode status = dataMgr->objectParent(pReg, pParentReg);
        if ( status.isSuccess() )  {
          IOpaqueAddress* pParAddr = pParentReg->address();
          if ( pParAddr )   {
            TDirectory* pParentDir = (TDirectory*)pParAddr->ipar()[0];
            if ( pParentDir )   {
              gDirectory->cd(pParentDir->GetPath());
              return pParentDir;
            }
          }
        }
      }
    }
  }
  return 0;
}

//-----------------------------------------------------------------------------
/// Convert the transient object to the requested representation.
StatusCode RootHistCnv::RConverter::createRep(DataObject* pObject,
                                              IOpaqueAddress*& pAddr)
//-----------------------------------------------------------------------------
{
  GlobalDirectoryRestore restore;
  pAddr = 0;
  try   {
    TDirectory* pParentDir = changeDirectory(pObject);
    if ( pParentDir )   {
      TObject* pTObj = createPersistent(pObject);
      if ( pTObj )   {
        pTObj->Write();
        delete pTObj;
        return createAddress(pObject, pParentDir, 0, pAddr);
      }
    }
  }
  catch (...)   {
  }
  MsgStream log (msgSvc(), "RConverter");
  log << MSG::ERROR << "Failed to create persistent Object!" << endmsg;
  return StatusCode::FAILURE;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::readObject(IOpaqueAddress* /* pAddr */ ,
                                               DataObject*&   /* refpObj */ )
{
//    MsgStream log(msgSvc(), "RConverter::readObject");
//    log << MSG::WARNING << pAddr->par()[0] << " <> " << pAddr->par()[1]
//        << " <> "
//        << pAddr->ipar()[0] << " <> " << pAddr->ipar()[1] << " <> "
//        << pAddr->registry()->identifier() << endmsg;

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
TObject* RootHistCnv::RConverter::createPersistent(DataObject*   /* pObj */)
{
  return 0;
}


//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::regTFile(const std::string id,
					     const TFile* tfile)
//-----------------------------------------------------------------------------
{

  MsgStream log(msgSvc(), "RConverter");

  std::map<std::string,TFile*>::const_iterator imap;
  imap = s_fileMap.find(id);

  if ( imap != s_fileMap.end() ) {
    log << MSG::ERROR << "cannot register TTree " << id
        << ": already exists" << endmsg;
    return StatusCode::FAILURE;
  }

  s_fileMap[id] = const_cast<TFile*>(tfile);

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::findTFile(const std::string id,
					      TFile*& tfile)
//-----------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), "RConverter");
  tfile = 0;

  std::string idm;

  // make sure we only get first two parts of id
  int i1,i2,i3;
  i1 = id.find("/",0);
  if (i1 != 0) {
    log << MSG::ERROR << "Directory name does not start with \"/\": "
	<< id << endmsg;
    return StatusCode::FAILURE;
  }
  i2 = id.find("/",i1+1);
  if (i2 == -1) {
    log << MSG::ERROR << "Directory name has only one part: " << id << endmsg;
    return StatusCode::FAILURE;
  }
  i3 = id.find("/",i2+1);
  if (i3 == -1) {
    idm = id;
  } else {
    idm = id.substr(0,i3);
  }

  std::map<std::string,TFile*>::const_iterator imap;
  imap = s_fileMap.find(idm);

  if ( imap == s_fileMap.end() ) {
    return StatusCode::FAILURE;
  }
  tfile = (*imap).second;
  return StatusCode::SUCCESS;
}
//-----------------------------------------------------------------------------
std::string RootHistCnv::RConverter::convertId(const std::string& id ) const
//-----------------------------------------------------------------------------
{
  bool forced = false;
  if ( id.size() > 0 && isdigit(id[0]) ) {
    try {
      BooleanProperty tmp;
      tmp.assign(SmartIF<IProperty>(conversionSvc())->getProperty( "ForceAlphaIds"));
      forced = (bool)tmp;
    }
    catch ( ... ) { }
  }
  if (forced )  return std::string("h") + id;
  else          return id;
}
//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RConverter::error(const std::string& msg)
//-----------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), "RootHistCnv");
  log << MSG::ERROR << msg << endmsg;
  return StatusCode::FAILURE;
}

