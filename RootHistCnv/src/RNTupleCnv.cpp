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
#include "RNTupleCnv.h"
#include "RootObjAddress.h"

#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/INTupleSvc.h>
#include <GaudiKernel/IOpaqueAddress.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/NTuple.h>
#include <GaudiKernel/SmartIF.h>

// Root
#include <TTree.h>

//-----------------------------------------------------------------------------
//
// Implementation of class :  RootHistCnv::RNTupleCnv
//
// Author :                   Charles Leggett
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Standard constructor
RootHistCnv::RNTupleCnv::RNTupleCnv( ISvcLocator* svc, const CLID& clid ) : RConverter( clid, svc ) {}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Initialize the converter
StatusCode RootHistCnv::RNTupleCnv::initialize() {
  //-----------------------------------------------------------------------------
  StatusCode status = Converter::initialize();
  if ( status.isSuccess() ) {
    m_ntupleSvc = serviceLocator()->service( "NTupleSvc" );
    if ( !m_ntupleSvc ) status = StatusCode::FAILURE;
  }
  return status;
}

//-----------------------------------------------------------------------------
/// Finalize the converter
StatusCode RootHistCnv::RNTupleCnv::finalize() {
  //-----------------------------------------------------------------------------
  m_ntupleSvc.reset();
  return Converter::finalize();
}

//-----------------------------------------------------------------------------
/// Update the transient object from the other representation.
StatusCode RootHistCnv::RNTupleCnv::updateObj( IOpaqueAddress* pAddress, DataObject* pObject )
//-----------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), "RNTupleCnv" );

  StatusCode status = StatusCode::FAILURE;

  RootObjAddress* rAddr = dynamic_cast<RootObjAddress*>( pAddress );

  if ( !rAddr ) {
    log << MSG::ERROR << "Could not dynamic cast to RootObjAddress" << endmsg;
    return StatusCode::FAILURE;
  }

  TTree* rtree = (TTree*)rAddr->tObj();

  try {
    unsigned long* info = (unsigned long*)pAddress->ipar();
    setDirectory( pAddress->par()[0] );
    status = readData( rtree, dynamic_cast<INTuple*>( pObject ), info[1]++ );
  } catch ( ... ) {}
  return status;
}

//-----------------------------------------------------------------------------
/// Create the transient representation of an object.
StatusCode RootHistCnv::RNTupleCnv::createObj( IOpaqueAddress* pAddress, DataObject*& refpObject )
//-----------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), "RNTupleCnv" );

  IRegistry* pReg = pAddress->registry();

  //    log << MSG::WARNING << "adr: " << pAddress->par()[0] << " <> "
  //        << pAddress->par()[1] << " <> " << pAddress->ipar()[0] << " <> "
  //        << pAddress->ipar()[1] << " <> " << hex << rAddr->tObj()
  //        << dec << " <> " << pReg->identifier() << endmsg;

  std::string ident = pReg->identifier();

  StatusCode status = readObject( pAddress, refpObject ); // Doesn't do anything
  if ( status.isSuccess() ) {
    RootObjAddress* rAddr = dynamic_cast<RootObjAddress*>( pAddress );
    if ( !rAddr ) {
      log << MSG::ERROR << "Could not cast to RootObjAddress" << endmsg;
      return StatusCode::FAILURE;
    }
    INTuple* nt   = nullptr;
    TTree*   tobj = (TTree*)rAddr->tObj();
    status        = load( tobj, nt );
    if ( status.isSuccess() ) {
      refpObject = dynamic_cast<DataObject*>( nt );
    } else {
      log << MSG::ERROR << "Problems loading ntuple id: " << pReg->identifier() << endmsg;
    }
  }
  return status;
}

//-----------------------------------------------------------------------------
/// Convert the transient object to the requested representation.
StatusCode RootHistCnv::RNTupleCnv::createRep( DataObject* pObject, IOpaqueAddress*& pAddr )
//-----------------------------------------------------------------------------
{
  GlobalDirectoryRestore restore;
  pAddr = nullptr;
  try {
    IRegistry* pReg = pObject->registry();
    if ( pReg ) {
      pAddr = pReg->address();
      if ( !pAddr ) {
        auto dataMgr = dataProvider().as<IDataManagerSvc>();
        if ( dataMgr ) {
          IRegistry* pParentReg = nullptr;
          StatusCode status     = dataMgr->objectParent( pReg, pParentReg );
          if ( status.isSuccess() ) {
            IOpaqueAddress* pParAddr = pParentReg->address();
            if ( pParAddr ) {
              TDirectory* pParentDir = (TDirectory*)pParAddr->ipar()[0];
              if ( pParentDir ) {
                TTree* pTree = nullptr;
                auto   dsc   = pReg->name().substr( 1 );
                gDirectory   = pParentDir;
                status       = book( dsc, dynamic_cast<INTuple*>( pObject ), pTree );
                if ( !status.isSuccess() ) { return status; }
                status = createAddress( pObject, gDirectory, pTree, pAddr );
                if ( !status.isSuccess() ) { return status; }
                return writeData( pTree, dynamic_cast<INTuple*>( pObject ) );
              }
            }
          }
        }
      } else {
        TDirectory*     pDir  = (TDirectory*)pAddr->ipar()[0];
        RootObjAddress* rAddr = dynamic_cast<RootObjAddress*>( pAddr );
        if ( !rAddr ) {
          MsgStream log( msgSvc(), "RNTupleCnv" );
          log << MSG::ERROR << "Could not cast to RootObjAddress" << endmsg;
          return StatusCode::FAILURE;
        }
        TTree* pTree = (TTree*)rAddr->tObj();
        gDirectory   = pDir;
        return writeData( pTree, dynamic_cast<INTuple*>( pObject ) );
      }
    }
  } catch ( ... ) {}
  MsgStream log( msgSvc(), "RNTupleCnv" );
  log << MSG::ERROR << "Failed to create persistent N-tuple!" << endmsg;
  return StatusCode::FAILURE;
}

//-----------------------------------------------------------------------------
/// Update the converted representation of a transient object.
StatusCode RootHistCnv::RNTupleCnv::updateRep( IOpaqueAddress* pAddr, DataObject* pObj )
//-----------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), "RNTupleCnv" );
  if ( pAddr ) {
    GlobalDirectoryRestore restore;
    TDirectory*            pDir  = (TDirectory*)pAddr->ipar()[0];
    RootObjAddress*        rAddr = dynamic_cast<RootObjAddress*>( pAddr );
    if ( !rAddr ) {
      log << MSG::ERROR << "Could not cast to RootObjAddress" << endmsg;
      return StatusCode::FAILURE;
    }
    TTree* pTree = (TTree*)rAddr->tObj();
    if ( pDir && pTree ) {
      gDirectory->cd( pDir->GetPath() );
      pTree->Write( "", TObject::kOverwrite );
      return StatusCode::SUCCESS;
    }
  } else {
    log << MSG::WARNING << "empty ntuple: " << pObj->registry()->identifier() << endmsg;
    return createRep( pObj, pAddr );
  }
  return StatusCode::FAILURE;
}

/// Return ROOT type info:

//-----------------------------------------------------------------------------
std::string RootHistCnv::RNTupleCnv::rootVarType( int type ) {
  //-----------------------------------------------------------------------------
  switch ( type ) {
  case DataTypeInfo::BOOL:
    return "/O"; // bool
  case DataTypeInfo::SHORT:
    return "/S"; // short
  case DataTypeInfo::INT:
    return "/I"; // int
  case DataTypeInfo::LONG:
    return "/I"; // long
  case DataTypeInfo::LONGLONG:
    return "/L"; // long long
  case DataTypeInfo::USHORT:
    return "/s"; // ushort
  case DataTypeInfo::UINT:
    return "/i"; // uint
  case DataTypeInfo::ULONG:
    return "/i"; // ulong
  case DataTypeInfo::ULONGLONG:
    return "/l"; // unsigned long long
  case DataTypeInfo::FLOAT:
    return "/F"; // float
  case DataTypeInfo::DOUBLE:
    return "/D"; // double
  case DataTypeInfo::CHAR:
    return "/B"; // char
  case DataTypeInfo::UCHAR:
    return "/b"; // char
  default:
    return "";
  }
  // cannot reach this point
}

//-----------------------------------------------------------------------------
bool RootHistCnv::parseName( const std::string& full, std::string& blk, std::string& var ) {
  //-----------------------------------------------------------------------------
  int sp = -1;
  if ( ( sp = full.find( "/" ) ) != -1 ) {
    blk = full.substr( 0, sp );
    var = full.substr( sp + 1 );
    return true;
  } else {
    blk = "AUTO_BLK";
    var = full;
    return false;
  }
}

//-----------------------------------------------------------------------------
/// Make the compiler instantiate the code...

#define INSTANTIATE( TYP )                                                                                             \
  template INTupleItem* createNTupleItem<TYP>( const std::string& itemName, const std::string& blockName,              \
                                               const std::string& index_name, int indexRange, int arraySize,           \
                                               TYP minimum, TYP maximum, INTuple* tuple, bool hasRange )

namespace RootHistCnv {

  template <class TYP>
  INTupleItem* createNTupleItem( const std::string& itemName, const std::string& blockName,
                                 const std::string& indexName, int indexRange, int arraySize, TYP min, TYP max,
                                 INTuple* ntup, bool hasRange ) {

    std::string varName;
    if ( blockName != "" ) {
      varName = blockName + "/" + itemName;
    } else {
      varName = itemName;
    }

    TYP          null = 0;
    INTupleItem* col  = nullptr;

    if ( !hasRange ) {
      min = NTuple::Range<TYP>::min();
      max = NTuple::Range<TYP>::max();
    }

    if ( indexName == "" ) {

      if ( arraySize == 1 ) {
        // simple items
        col = NTuple::_Item<TYP>::create( ntup, varName, typeid( TYP ), min, max, null );

      } else {
        // Arrays of fixed size
        col = NTuple::_Array<TYP>::create( ntup, varName, typeid( TYP ), "", arraySize, min, max, null );
      }

    } else {

      if ( arraySize == 1 ) {
        // Arrays of variable size
        col = NTuple::_Array<TYP>::create( ntup, varName, typeid( TYP ), indexName, indexRange, min, max, null );
      } else {
        // Matrices
        col = NTuple::_Matrix<TYP>::create( ntup, varName, typeid( TYP ), indexName, indexRange, arraySize, min, max,
                                            null );
      }
    }

    return col;
  }

  INSTANTIATE( float );
  INSTANTIATE( double );
  INSTANTIATE( bool );
  INSTANTIATE( char );
  INSTANTIATE( int );
  INSTANTIATE( short );
  INSTANTIATE( long );
  INSTANTIATE( long long );
  INSTANTIATE( unsigned char );
  INSTANTIATE( unsigned int );
  INSTANTIATE( unsigned short );
  INSTANTIATE( unsigned long );
  INSTANTIATE( unsigned long long );
} // namespace RootHistCnv
