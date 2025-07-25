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
// Include files
#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/IOpaqueAddress.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/NTuple.h>

#include <optional>

#include "RDirectoryCnv.h"

// Root files
#include <TDirectory.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TKey.h>
#include <TObject.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TTree.h>

namespace {
  auto maybe_stol = []( const std::string& s ) -> std::optional<int> {
    auto pos = s.find_first_of( "0123456789+-" );
    if ( pos == std::string::npos ) return std::nullopt;
    return std::stol( s.substr( pos ) );
  };
} // namespace

DECLARE_CONVERTER( RootHistCnv::RDirectoryCnv )

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RDirectoryCnv::createObj( IOpaqueAddress* /* pAddress */, DataObject*& refpObject )
//-----------------------------------------------------------------------------
{
  refpObject = new NTuple::Directory();
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RDirectoryCnv::createRep( DataObject* pObject, IOpaqueAddress*& refpAddress )
//-----------------------------------------------------------------------------
{
  const std::string& loc = pObject->registry()->identifier();
  if ( createDirectory( loc ).isSuccess() ) {
    setDirectory( loc );
    setDiskDirectory( loc );
    //  return createAddress(pObject, pObject->registry()->name(), refpAddress);
    return createAddress( pObject, gDirectory, nullptr, refpAddress );
  }
  refpAddress = nullptr;
  return StatusCode::FAILURE;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RDirectoryCnv::updateRep( IOpaqueAddress* /* pAddress */, DataObject* pObject )
//-----------------------------------------------------------------------------
{
  const std::string& loc = pObject->registry()->identifier();
  if ( createDirectory( loc ).isSuccess() ) {
    setDirectory( loc );
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RDirectoryCnv::fillObjRefs( IOpaqueAddress* pAddr, DataObject* pObj ) {
  MsgStream          log( msgSvc(), "RDirectoryCnv" );
  IRegistry*         pReg  = pObj->registry();
  std::string        full  = pReg->identifier();
  const std::string& fname = pAddr->par()[0];

  TFile* tf = nullptr;
  findTFile( full, tf ).ignore();

  // cd to TFile:
  setDirectory( full );
  TIter nextkey( gDirectory->GetListOfKeys() );
  while ( TKey* key = (TKey*)nextkey() ) {
    IOpaqueAddress* pA    = nullptr;
    TObject*        obj   = key->ReadObj();
    std::string     title = obj->GetTitle();
    std::string     sid   = obj->GetName();
    std::string     f2    = full + "/" + sid;
    int             idh   = maybe_stol( sid ).value_or( 0 );
    // introduced by Grigori Rybkine
    std::string clname = key->GetClassName();
    std::string clnm   = clname.substr( 0, 3 );
    TClass*     isa    = obj->IsA();
    if ( isa->InheritsFrom( "TTree" ) ) {
      createAddress( full, CLID_ColumnWiseTuple, idh, obj, pA ).ignore();
      TTree* tree = (TTree*)obj;
      tree->Print();
      log << MSG::DEBUG << "Reg CWNT \"" << obj->GetTitle() << "\" as " << f2 << endmsg;
      title = "/" + sid;
    } else if ( isa->InheritsFrom( "TDirectory" ) ) {
      createAddress( full, CLID_NTupleDirectory, title, obj, pA ).ignore();
    } else if ( isa == TProfile::Class() ) {
      createAddress( full, CLID_ProfileH, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TProfile2D::Class() ) {
      createAddress( full, CLID_ProfileH2, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TH1C::Class() ) {
      createAddress( full, CLID_H1D, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TH1S::Class() ) {
      createAddress( full, CLID_H1D, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TH1I::Class() ) {
      createAddress( full, CLID_H1D, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TH1F::Class() ) {
      createAddress( full, CLID_H1D, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TH1D::Class() ) {
      createAddress( full, CLID_H1D, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TH2C::Class() ) {
      createAddress( full, CLID_H2D, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TH2S::Class() ) {
      createAddress( full, CLID_H2D, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TH2I::Class() ) {
      createAddress( full, CLID_H2D, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TH2F::Class() ) {
      createAddress( full, CLID_H2D, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TH2D::Class() ) {
      createAddress( full, CLID_H2D, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TH3C::Class() ) {
      createAddress( full, CLID_H3D, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TH3S::Class() ) {
      createAddress( full, CLID_H3D, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TH3I::Class() ) {
      createAddress( full, CLID_H3D, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TH3F::Class() ) {
      createAddress( full, CLID_H3D, idh, obj, pA ).ignore();
      title = sid;
    } else if ( isa == TH3D::Class() ) {
      createAddress( full, CLID_H3D, idh, obj, pA ).ignore();
      title = sid;
    } else {
      log << MSG::ERROR << "Encountered an unknown object with key: " << obj->GetName() << " in ROOT file " << fname
          << endmsg;
      return StatusCode::FAILURE;
    }
    if ( pA ) {
      StatusCode sc = dataManager()->registerAddress( pReg, title, pA );
      if ( !sc.isSuccess() ) {
        log << MSG::ERROR << "Failed to register address for " << full << endmsg;
        return sc;
      }
      log << MSG::VERBOSE << "Created address for " << clnm << "'" << title << "' in " << full << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}
