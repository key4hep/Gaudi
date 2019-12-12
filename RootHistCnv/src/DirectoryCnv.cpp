/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// Include files
#include "DirectoryCnv.h"
//------------------------------------------------------------------------------
//
// Implementation of class :  RootHistCnv::DirectoryCnv
//
// Author :                   Charles Leggett
//
//------------------------------------------------------------------------------
DECLARE_CONVERTER( RootHistCnv::DirectoryCnv )
//------------------------------------------------------------------------------
StatusCode RootHistCnv::DirectoryCnv::createObj( IOpaqueAddress* /* pAddress */, DataObject*& refpObject ) {
  refpObject = new DataObject();
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode RootHistCnv::DirectoryCnv::createRep( DataObject* pObj, IOpaqueAddress*& refpAddr ) {
  if ( changeDirectory( pObj ) ) {
    const char* d_nam = pObj->name().c_str() + 1;
    if ( !gDirectory->GetKey( d_nam ) ) { gDirectory->mkdir( d_nam ); }
    gDirectory->cd( d_nam );
    return createAddress( pObj, gDirectory, nullptr, refpAddr );
  }
  refpAddr = nullptr;
  return StatusCode::FAILURE;
}
