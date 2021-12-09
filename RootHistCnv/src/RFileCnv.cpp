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
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"

#include "Gaudi/Interfaces/IOptionsSvc.h"

// ROOT
#include "RFileCnv.h"
#include "TFile.h"
#include "TROOT.h"

// local
#include "RootCompressionSettings.h"
#include <GaudiKernel/IMetaDataSvc.h>
#include <map>
#include <string>

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_CONVERTER( RootHistCnv::RFileCnv )

// Standard constructor
RootHistCnv::RFileCnv::RFileCnv( ISvcLocator* svc ) : RDirectoryCnv( svc, classID() ) {}

//------------------------------------------------------------------------------
StatusCode RootHistCnv::RFileCnv::initialize() {
  // Set compression level property ...
  const auto& optsSvc = serviceLocator()->getOptsSvc();
  if ( optsSvc.has( "RFileCnv.GlobalCompression" ) ) {
    auto sc = Gaudi::Parsers::parse( m_compLevel, optsSvc.get( "RFileCnv.GlobalCompression" ) );
    if ( !sc ) return sc;
  }
  // initialise base class
  return RDirectoryCnv::initialize();
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
StatusCode RootHistCnv::RFileCnv::createObj( IOpaqueAddress* pAddress, DataObject*& refpObject )
//------------------------------------------------------------------------------
{
  MsgStream      log( msgSvc(), "RFileCnv" );
  unsigned long* ipar    = (unsigned long*)pAddress->ipar();
  char           mode[2] = { char( ipar[1] ), 0 };

  std::string fname  = pAddress->par()[0]; // Container name
  std::string ooname = pAddress->par()[1]; // Object name

  const std::string* spar = pAddress->par();
  // Strip of store name to get the top level RZ directory
  std::string oname = spar[1].substr( spar[1].find( "/", 1 ) + 1 );

  // Protect against multiple instances of TROOT
  if ( !gROOT ) {
    static TROOT root( "root", "ROOT I/O" );
    //    gDebug = 99;
  } else {
    log << MSG::VERBOSE << "ROOT already initialized, debug = " << gDebug << endmsg;
  }

  // Determine access mode:

  if ( mode[0] == 'O' ) {

    if ( findTFile( ooname, rfile ).isFailure() ) {

      log << MSG::INFO << "opening Root file \"" << fname << "\" for reading" << endmsg;

      rfile = TFile::Open( fname.c_str(), "READ" );
      if ( rfile && rfile->IsOpen() ) {
        regTFile( ooname, rfile ).ignore();

        ipar[0]             = (unsigned long)rfile;
        NTuple::File* pFile = new NTuple::File( objType(), fname, oname );
        pFile->setOpen( true );
        refpObject = pFile;

        return StatusCode::SUCCESS;

      } else {
        log << MSG::ERROR << "Couldn't open \"" << fname << "\" for reading" << endmsg;
        return StatusCode::FAILURE;
      }

    } else {
      log << MSG::DEBUG << "Root file \"" << fname << "\" already opened" << endmsg;
      return StatusCode::SUCCESS;
    }

  } else if ( mode[0] == 'U' ) {

    log << MSG::INFO << "opening Root file \"" << fname << "\" for updating" << endmsg;

    log << MSG::ERROR << "don't know how to do this yet. Aborting." << endmsg;
    exit( 1 );

  } else if ( mode[0] == 'N' ) {

    log << MSG::INFO << "opening Root file \"" << fname << "\" for writing";
    if ( !m_compLevel.empty() ) { log << ", CompressionLevel='" << m_compLevel << "'"; }
    log << endmsg;

    rfile = TFile::Open( fname.c_str(), "RECREATE", "Gaudi Trees" );
    if ( !( rfile && rfile->IsOpen() ) ) {
      log << MSG::ERROR << "Could not open file " << fname << " for writing" << endmsg;
      return StatusCode::FAILURE;
    }
    if ( !m_compLevel.empty() ) {
      const RootCompressionSettings settings( m_compLevel );
      rfile->SetCompressionSettings( settings.level() );
    }

    regTFile( ooname, rfile ).ignore();

    log << MSG::DEBUG << "creating ROOT file " << fname << endmsg;

    ipar[0]             = (unsigned long)rfile;
    NTuple::File* pFile = new NTuple::File( objType(), fname, oname );
    pFile->setOpen( true );
    refpObject = pFile;
    return StatusCode::SUCCESS;

  } else {

    log << MSG::ERROR << "Uknown mode to access ROOT file" << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::FAILURE;
}

//------------------------------------------------------------------------------
StatusCode RootHistCnv::RFileCnv::createRep( DataObject* pObject, IOpaqueAddress*& refpAddress )
//------------------------------------------------------------------------------
{
  refpAddress = pObject->registry()->address();
  return RFileCnv::updateRep( refpAddress, pObject );
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RFileCnv::updateRep( IOpaqueAddress* pAddress, DataObject* pObject )
//-----------------------------------------------------------------------------
{
  MsgStream   log( msgSvc(), "RFileCnv" );
  std::string ooname = pAddress->par()[1];

  NTuple::File* pFile = dynamic_cast<NTuple::File*>( pObject );
  if ( pFile && pFile->isOpen() ) {

    unsigned long* ipar = (unsigned long*)pAddress->ipar();
    if ( findTFile( ooname, rfile ).isFailure() ) {
      log << MSG::ERROR << "Problems closing TFile " << ooname << endmsg;
      return StatusCode::FAILURE;
    }

    rfile->Write( nullptr, TObject::kOverwrite );
    if ( log.level() <= MSG::INFO ) {
      log << MSG::INFO << "dumping contents of " << ooname << endmsg;
      rfile->Print();
    }

    /*
     * MetaData
     * Ana Trisovic
     * March 2015
     * */
    SmartIF<IMetaDataSvc> mds;
    mds = serviceLocator()->service( "Gaudi::MetaDataSvc", false );
    // auto mds = service<IMetaDataSvc>("MetaDataSvc", false);
    if ( mds ) {
      std::map<std::string, std::string> m_metadata = mds->getMetaDataMap();
      if ( !rfile->WriteObject( &m_metadata, "info" ) ) { return StatusCode::FAILURE; }
    }
    /* */

    rfile->Close();
    delete rfile;

    ipar[0] = 0;
    pFile->setOpen( false );
    return StatusCode::SUCCESS;

  } else {
    log << MSG::ERROR << "TFile " << ooname << " is not open" << endmsg;
  }
  return StatusCode::FAILURE;
}
