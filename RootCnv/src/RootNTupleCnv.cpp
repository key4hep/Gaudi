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
//------------------------------------------------------------------------------
//
// Implementation of class :  RootNTupleCnv
//
// Author :                   Markus Frank
//
//------------------------------------------------------------------------------

#define ALLOW_ALL_TYPES
// Include files
#include <RootCnv/RootAddress.h>
#include <RootCnv/RootDataConnection.h>
#include <RootCnv/RootNTupleCnv.h>
#include <RootCnv/RootRefs.h>

#include <GaudiKernel/ContainedObject.h>
#include <GaudiKernel/INTupleSvc.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/ISelectStatement.h>
#include <GaudiKernel/NTuple.h>
#include <GaudiKernel/SmartIF.h>
#include <GaudiKernel/SmartRef.h>
#include <GaudiKernel/StreamBuffer.h>

// ROOT include files
#include <TBranch.h>
#include <TROOT.h>
#include <TTree.h>

#include <limits>
#include <memory>

#define S_OK StatusCode::SUCCESS
#define S_FAIL StatusCode::FAILURE

using namespace Gaudi;
using namespace std;

static inline istream& loadLong( istream& is ) {
  long i;
  is >> i;
  return is;
}
static inline istream& operator>>( istream& is, IOpaqueAddress*& /*pObj*/ ) { return loadLong( is ); }

#if 0
static inline istream& operator>>(istream& is, SmartRef<DataObject>& /*pObj*/)
{  return loadLong(is);          }
static inline istream& operator>>(istream& is, SmartRef<ContainedObject>& /*pObj*/)
{  return loadLong(is);          }
static inline istream& operator>>(istream& is, string& /*pObj*/)
{  return loadLong(is);          }
#endif

template <class TYP>
static StatusCode createItem( TTree* tree, INTuple* tuple, istream& is, const string& name, bool add,
                              const TYP& null ) {
  string       idxName;
  long         len, ndim, dim[4], hasIdx, idxLow, idxLen;
  long         dim1 = 1, dim2 = 1;
  INTupleItem* it = nullptr;
  char         c;
  is >> len >> c >> ndim >> c >> hasIdx >> c;
  if ( hasIdx ) { getline( is, idxName, ';' ) >> idxLow >> c >> idxLen >> c; }
  for ( int i = 0; i < ndim; i++ ) is >> dim[i] >> c;

  TYP low = null, high = null;
  is >> low >> c >> high >> c;
  is >> c;
  switch ( ndim ) {
  case 0:
    it = NTuple::_Item<TYP>::create( tuple, name, typeid( TYP ), low, high, null );
    break;
  case 1:
    dim1 = ( hasIdx ) ? idxLen : dim[0];
    it   = NTuple::_Array<TYP>::create( tuple, name, typeid( TYP ), idxName, dim1, low, high, null );
    break;
  case 2:
    dim1 = ( hasIdx ) ? idxLen : dim[0];
    dim2 = ( hasIdx ) ? dim[0] : dim[1];
    it   = NTuple::_Matrix<TYP>::create( tuple, name, typeid( TYP ), idxName, dim1, dim2, low, high, null );
    break;
  default:
    return S_FAIL;
  }
  if ( add ) {
    TBranch* b = tree->GetBranch( it->name().c_str() );
    if ( b ) {
      b->SetAddress( const_cast<void*>( it->buffer() ) );
    } else {
      // return StatusCode::SUCCESS;
      return tuple->add( it );
    }
  }
  return tuple->add( it );
}

template <class T>
static inline void putRange( ostream& os, NTuple::_Data<T>* it ) {
  const NTuple::Range<T>& x = it->range();
  os << x.lower() << ';' << x.upper() << ';';
}

static inline string _tr( string s ) {
  string local = std::move( s );
  auto   p     = std::begin( local );
  if ( local.compare( 0, 7, "<local>" ) == 0 ) p += 7;
  std::replace_if(
      p, std::end( local ), []( const char& c ) { return !isalnum( c ); }, '_' );
  return local;
}

// Converter overrides: Update the references of an updated transient object.
StatusCode RootNTupleCnv::createObj( IOpaqueAddress* pAddr, DataObject*& refpObject ) {
  StatusCode          status    = S_FAIL;
  RootDataConnection* con       = nullptr;
  IRegistry*          pRegistry = pAddr->registry();
  RootAddress*        rpA       = dynamic_cast<RootAddress*>( pAddr );
  string              path      = fileName( pRegistry );
  string              cntName   = containerName( pRegistry );
  string*             par       = const_cast<string*>( pAddr->par() );
  status                        = m_dbMgr->connectDatabase( path, IDataConnection::READ, &con );
  if ( status.isSuccess() ) {
    string   par_val, par_guid;
    TBranch* b = con->getBranch( "##Descriptors", "GaudiStatisticsDescription" );
    if ( b ) {
      RootNTupleDescriptor*                 ptr;
      std::unique_ptr<RootNTupleDescriptor> dsc( ptr = new RootNTupleDescriptor() );
      b->SetAddress( &ptr );
      for ( Long64_t i = 0, nent = b->GetEntries(); i < nent; ++i ) {
        int nb = b->GetEntry( i );
        if ( nb > 1 ) {
          if ( ptr->container == cntName ) {
            par_val = ptr->description;
            break;
          }
        }
      }
    }
    par[2]      = _tr( cntName );
    TTree* tree = con->getSection( par[2] );
    if ( nullptr == tree ) { return makeError( "Failed to access N-Tuple tree:" + cntName ); }
    if ( !par_val.empty() ) {
      auto ntupleSvc = dataProvider().as<INTupleSvc>();
      if ( ntupleSvc ) {
        char           c;
        CLID           clid;
        int            siz, typ;
        string         title;
        NTuple::Tuple* nt = nullptr;
        istringstream  is( par_val );
        getline( is, title, ';' ) >> clid >> c >> siz >> c;
        status = ntupleSvc->create( clid, title, nt );
        for ( int j = 0; j < siz && status.isSuccess(); j++ ) {
          is >> c;
          getline( is, title, ';' ) >> typ >> c;
          switch ( typ ) {
          case DataTypeInfo::UCHAR:
            status = createItem( tree, nt, is, title, true, (unsigned char)0 );
            break;
          case DataTypeInfo::USHORT:
            status = createItem( tree, nt, is, title, true, (unsigned short)0 );
            break;
          case DataTypeInfo::UINT:
            status = createItem( tree, nt, is, title, true, 0u );
            break;
          case DataTypeInfo::ULONG:
            status = createItem( tree, nt, is, title, true, 0ul );
            break;
          case DataTypeInfo::CHAR:
            status = createItem( tree, nt, is, title, true, char( 0 ) );
            break;
          case DataTypeInfo::SHORT:
            status = createItem( tree, nt, is, title, true, short( 0 ) );
            break;
          case DataTypeInfo::INT:
            status = createItem( tree, nt, is, title, true, 0 );
            break;
          case DataTypeInfo::LONG:
            status = createItem( tree, nt, is, title, true, 0l );
            break;
          case DataTypeInfo::BOOL:
            status = createItem( tree, nt, is, title, true, false );
            break;
          case DataTypeInfo::FLOAT:
            status = createItem( tree, nt, is, title, true, 0.f );
            break;
          case DataTypeInfo::DOUBLE:
            status = createItem( tree, nt, is, title, true, 0. );
            break;
          /*
          case DataTypeInfo::NTCHAR:
          case DataTypeInfo::LONG_NTCHAR:
          status =
          createItem(tree, nt, is, title, true, (char*)0);
          break;
          case DataTypeInfo::STRING:
          case DataTypeInfo::LONG_STRING:
          status =
          createItem(tree, nt, is, title, true, string(""));
          break;
          */
          case DataTypeInfo::OBJECT_ADDR:
            status = createItem( tree, nt, is, title, false, static_cast<IOpaqueAddress*>( nullptr ) );
            break;
          case DataTypeInfo::POINTER:
            status = createItem( tree, nt, is, title, true, static_cast<void*>( nullptr ) );
            break;
          case DataTypeInfo::UNKNOWN:
          default:
            status = S_FAIL;
            break;
          }
          if ( !status.isSuccess() ) {
            log() << MSG::FATAL << "Error connecting (Unknown) column:" << j << endmsg << par_val << endmsg;
            return makeError( "createObj[NTuple]> Cannot determine column!" );
          }
        }
        if ( status.isSuccess() && rpA ) {
          unsigned long* ipar = const_cast<unsigned long*>( rpA->ipar() );
          log() << MSG::DEBUG << "Created N-tuple with description:" << par_val << endl;
          ipar[0]      = reinterpret_cast<uintptr_t>( con );
          ipar[1]      = ~0x0UL;
          rpA->section = tree;
          refpObject   = nt;
        } else {
          refpObject = nullptr;
          if ( nt ) nt->release();
        }
      }
    }
  }
  return status;
}

// Update the transient object: NTuples end here when reading records
StatusCode RootNTupleCnv::updateObj( IOpaqueAddress* pAddr, DataObject* pObj ) {
  INTuple*     tupl = dynamic_cast<INTuple*>( pObj );
  RootAddress* rpA  = dynamic_cast<RootAddress*>( pAddr );
  if ( !tupl || !rpA ) return makeError( "updateObj> Invalid Tuple reference." );
  RootDataConnection* con = reinterpret_cast<RootDataConnection*>( rpA->ipar()[0] );
  if ( !con ) return makeError( "updateObj> Failed to access data source!" );
  TTree* tree = rpA->section;
  if ( !tree ) return makeError( "Failed to access data tree:" + pAddr->par()[1] );
  con->resetAge();
  if ( con->tool()->refs() ) return i__updateObjRoot( rpA, tupl, tree, con );
#ifdef __POOL_COMPATIBILITY
  // POOL compatibility mode:
  return i__updateObjPool( rpA, tupl, tree, con );
#else
  return makeError( "Failed to access reference branch for data tree:" + rpA->par()[1] );
#endif
}

// Update the transient object: NTuples end here when reading records
StatusCode RootNTupleCnv::i__updateObjRoot( RootAddress* rpA, INTuple* tupl, TTree* tree, RootDataConnection* con ) {
  typedef INTuple::ItemContainer Cont;
  const string*                  par  = rpA->par();
  unsigned long*                 ipar = const_cast<unsigned long*>( rpA->ipar() );
  ++ipar[1];
  if ( Long64_t( ipar[1] ) <= tree->GetEntries() ) {
    GenericAddress*  pA = nullptr;
    Cont&            it = tupl->items();
    size_t           k, n = it.size();
    vector<RootRef*> paddr( n );
    vector<RootRef>  addr( n );
    for ( k = 0; k < n; ++k ) {
      Cont::value_type j = it[k];
      switch ( j->type() ) {
      case DataTypeInfo::OBJECT_ADDR:
        paddr[k] = &addr[k];
        tree->SetBranchAddress( j->name().c_str(), &paddr[k] );
        break;
      default:
        break;
      }
    }

    ULong64_t         last = tree->GetEntries();
    ISelectStatement* sel  = tupl->selector();
    if ( sel ) {
      string criteria = ( sel && ( sel->type() & ISelectStatement::STRING ) ) ? sel->criteria() : string( "" );
      if ( !( criteria.length() == 0 || criteria == "*" ) ) {
        if ( rpA->select == nullptr ) {
          log() << MSG::DEBUG << "Selection criteria: " << criteria << "  " << ipar[1] << endmsg;
          rpA->select = new TTreeFormula( tree->GetName(), criteria.c_str(), tree );
        }
        rpA->select->SetTree( tree );
        for ( ; ipar[1] < last; ++ipar[1] ) { // loop on all selected entries
          tree->LoadTree( ipar[1] );
          rpA->select->GetNdata();
          if ( fabs( rpA->select->EvalInstance( 0 ) ) > std::numeric_limits<double>::epsilon() ) { break; }
          log() << MSG::DEBUG << par[0] << "/" << par[1] << " SKIP Entry: " << ipar[1] << endmsg;
        }
      }
    }
    if ( ipar[1] < last ) {
      unsigned long entry = ipar[1];
      if ( tree->GetEntry( entry ) > 1 ) {
        RootRef* r    = nullptr;
        string*  spar = nullptr;
        for ( k = 0; k < n; ++k ) {
          Cont::value_type j = it[k];
          switch ( j->type() ) {
          case DataTypeInfo::OBJECT_ADDR:
            r  = paddr[k];
            pA = ( *(GenericAddress**)j->buffer() );
            if ( pA ) { // Fill only if item is connected!
              spar = (string*)pA->par();
              ipar = (unsigned long*)pA->ipar();
              log() << MSG::DEBUG;
              pair<const RootRef*, const RootDataConnection::ContainerSection*> ls =
                  con->getMergeSection( tree->GetName(), entry );
              if ( ls.first ) {
                if ( ls.first->dbase >= 0 ) {
                  // Now patch the references and links 'en block' to be efficient
                  // First the leafs from the TES

                  r->dbase += ls.first->dbase;
                  r->container += ls.first->container;
                  r->link += ls.first->link;

                  if ( log().isActive() ) {
                    log() << "Refs: LS [" << entry << "] -> " << ls.first->dbase << "," << ls.first->container << ","
                          << ls.first->link << "," << ls.first->entry << " DB:" << con->getDb( r->dbase ) << endmsg;
                  }
                }
              }
              spar[0] = con->getDb( r->dbase );
              spar[1] = con->getCont( r->container );
              spar[2] = con->getLink( r->link );
              ipar[0] = 0;
              ipar[1] = r->entry;
              pA->setClID( r->clid );
              pA->setSvcType( r->svc );
              break;
            }
            break;
          default:
            break;
          }
        }
        return StatusCode::SUCCESS;
      }
      log() << MSG::ERROR << "Failed to read data from NTuple tree." << endmsg;
      return StatusCode::FAILURE;
    }
    log() << MSG::INFO << "End of input Ntuple." << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::FAILURE;
}

/// Convert the transient object to the requested representation.
StatusCode RootNTupleCnv::createRep( DataObject* pObj, IOpaqueAddress*& pAddr ) {
  IRegistry* pRegistry = pObj->registry();
  if ( pRegistry ) {
    pAddr = pRegistry->address();
    if ( pAddr ) return S_OK;

    RootDataConnection* con     = nullptr;
    string              path    = fileName( pRegistry );
    string              cntName = containerName( pRegistry );
    string              secName = cntName;
    const INTuple*      nt      = dynamic_cast<const INTuple*>( pObj );
    StatusCode          status  = m_dbMgr->connectDatabase( path, IDataConnection::UPDATE, &con );
    if ( !status.isSuccess() ) { return makeError( "Failed to access Tuple file:" + path ); }
    TTree* tree = con->getSection( _tr( secName ), true );
    if ( nullptr != nt ) {
      const INTuple::ItemContainer& items = nt->items();
      ostringstream                 os;
      size_t                        item_no;
      string                        desc;
      os << nt->title() << ';' << pObj->clID() << ';' << items.size() << ';';
      map<string, TBranch*> branches;
      TBranch*              b = nullptr;
      for ( item_no = 0; item_no < items.size(); ++item_no ) {
        INTupleItem* it = items[item_no];
        if ( it->hasIndex() ) {
          INTupleItem*  itm = it->indexItem();
          const string& n   = itm->name();
          switch ( itm->type() ) {
          case DataTypeInfo::UCHAR:
            desc = n + "/b";
            b    = tree->Branch( n.c_str(), const_cast<void*>( itm->buffer() ), desc.c_str() );
            break;
          case DataTypeInfo::USHORT:
            desc = n + "/s";
            b    = tree->Branch( n.c_str(), const_cast<void*>( itm->buffer() ), desc.c_str() );
            break;
          case DataTypeInfo::UINT:
            desc = n + "/i";
            b    = tree->Branch( n.c_str(), const_cast<void*>( itm->buffer() ), desc.c_str() );
            break;
          case DataTypeInfo::ULONG:
            desc = n + "/l";
            b    = tree->Branch( n.c_str(), const_cast<void*>( itm->buffer() ), desc.c_str() );
            break;
          case DataTypeInfo::CHAR:
            desc = n + "/B";
            b    = tree->Branch( n.c_str(), const_cast<void*>( itm->buffer() ), desc.c_str() );
            break;
          case DataTypeInfo::SHORT:
            desc = n + "/S";
            b    = tree->Branch( n.c_str(), const_cast<void*>( itm->buffer() ), desc.c_str() );
            break;
          case DataTypeInfo::INT:
            desc = n + "/I";
            b    = tree->Branch( n.c_str(), const_cast<void*>( itm->buffer() ), desc.c_str() );
            break;
          case DataTypeInfo::LONG:
            desc = n + "/L";
            b    = tree->Branch( n.c_str(), const_cast<void*>( itm->buffer() ), desc.c_str() );
            break;
          default:
            return makeError( "Column " + it->index() + " is not a valid index column!" );
          }
          branches[n] = b;
        }
      }
      for ( item_no = 0; item_no < items.size(); ++item_no ) {
        INTupleItem*  it = items[item_no];
        const string& n  = it->name();
        os << '{' << n << ';' << it->type() << ';' << it->length() << ';' << it->ndim() << ';' << it->hasIndex() << ';';
        if ( it->hasIndex() ) {
          os << it->index() << ';';
          INTupleItem* itm = it->indexItem();
          switch ( itm->type() ) {
          case DataTypeInfo::UCHAR:
            putRange( os, dynamic_cast<NTuple::_Data<unsigned char>*>( itm ) );
            break;
          case DataTypeInfo::USHORT:
            putRange( os, dynamic_cast<NTuple::_Data<unsigned short>*>( itm ) );
            break;
          case DataTypeInfo::UINT:
            putRange( os, dynamic_cast<NTuple::_Data<unsigned int>*>( itm ) );
            break;
          case DataTypeInfo::ULONG:
            putRange( os, dynamic_cast<NTuple::_Data<unsigned long>*>( itm ) );
            break;
          case DataTypeInfo::CHAR:
            putRange( os, dynamic_cast<NTuple::_Data<char>*>( itm ) );
            break;
          case DataTypeInfo::SHORT:
            putRange( os, dynamic_cast<NTuple::_Data<short>*>( itm ) );
            break;
          case DataTypeInfo::INT:
            putRange( os, dynamic_cast<NTuple::_Data<int>*>( itm ) );
            break;
          case DataTypeInfo::LONG:
            putRange( os, dynamic_cast<NTuple::_Data<long>*>( itm ) );
            break;
          default:
            return makeError( "NTuple:" + pRegistry->name() + " Column " + it->index() +
                              " is not a valid index column!" );
          }
        }
        for ( long k = 0; k < it->ndim(); k++ ) { os << it->dim( k ) << ';'; }
        desc       = n;
        TClass* cl = nullptr;
        switch ( it->type() ) {
        case DataTypeInfo::STRING:
          desc = "/C";
          os << 0 << ';' << 0 << ';';
          break;
        case DataTypeInfo::NTCHAR:
          desc = "/C";
          os << 0 << ';' << 0 << ';';
          break;
        case DataTypeInfo::OBJECT_ADDR:
          if ( it->length() == 1 ) {
            desc = System::typeinfoName( typeid( RootRef ) );
            os << 0 << ';' << 0 << ';';
            cl = TClass::GetClass( desc.c_str(), kTRUE );
          }
          break;
        case DataTypeInfo::POINTER:
          if ( it->length() == 1 ) {
            os << 0 << ';' << 0 << ';';
            cl = TClass::GetClass( it->typeID(), kTRUE );
          }
          break;
        case DataTypeInfo::UCHAR:
          desc = "/b";
          putRange( os, dynamic_cast<NTuple::_Data<unsigned char>*>( it ) );
          break;
        case DataTypeInfo::USHORT:
          desc = "/s";
          putRange( os, dynamic_cast<NTuple::_Data<unsigned short>*>( it ) );
          break;
        case DataTypeInfo::UINT:
          desc = "/i";
          putRange( os, dynamic_cast<NTuple::_Data<unsigned int>*>( it ) );
          break;
        case DataTypeInfo::ULONG:
          desc = "/l";
          putRange( os, dynamic_cast<NTuple::_Data<unsigned long>*>( it ) );
          break;
        case DataTypeInfo::CHAR:
          desc = "/B";
          putRange( os, dynamic_cast<NTuple::_Data<char>*>( it ) );
          break;
        case DataTypeInfo::SHORT:
          desc = "/S";
          putRange( os, dynamic_cast<NTuple::_Data<short>*>( it ) );
          break;
        case DataTypeInfo::INT:
          desc = "/I";
          putRange( os, dynamic_cast<NTuple::_Data<int>*>( it ) );
          break;
        case DataTypeInfo::LONG:
          desc = "/L";
          putRange( os, dynamic_cast<NTuple::_Data<long>*>( it ) );
          break;
        case DataTypeInfo::BOOL:
          desc = "/b";
          putRange( os, dynamic_cast<NTuple::_Data<bool>*>( it ) );
          break;
        case DataTypeInfo::FLOAT:
          desc = "/F";
          putRange( os, dynamic_cast<NTuple::_Data<float>*>( it ) );
          break;
        case DataTypeInfo::DOUBLE:
          desc = "/D";
          putRange( os, dynamic_cast<NTuple::_Data<double>*>( it ) );
          break;
        case DataTypeInfo::UNKNOWN:
        default:
          return makeError( "Create item[FAIL]: " + it->name() );
        }
        os << '}';
        if ( branches.find( n ) == branches.end() ) {
          string tmp;
          char   text[32];
          switch ( it->ndim() ) {
          case 0:
            desc = n + desc;
            break;
          case 2:
            sprintf( text, "[%ld]", it->dim( 0 ) );
            tmp = text;
            [[fallthrough]];
          case 1:
            if ( it->hasIndex() ) {
              INTupleItem* itm = it->indexItem();
              desc             = n + tmp + "[" + itm->name() + "]" + desc;
            } else {
              sprintf( text, "[%ld]", it->dim( 0 ) );
              desc = n + tmp + text + desc;
            }
          }
          log() << MSG::DEBUG << "Create branch:" << n << " Desc:" << desc << " of type:" << it->type() << endmsg;
          switch ( it->type() ) {
          case DataTypeInfo::OBJECT_ADDR:
            branches[n] = tree->Branch( n.c_str(), cl->GetName(), const_cast<void*>( it->buffer() ) );
            break;
          case DataTypeInfo::POINTER:
            branches[n] = tree->Branch( n.c_str(), cl->GetName(), const_cast<void*>( it->buffer() ) );
            break;
          default:
            branches[n] = tree->Branch( n.c_str(), const_cast<void*>( it->buffer() ), desc.c_str() );
            break;
          }
        }
      }

      log() << MSG::DEBUG << "Save description:" << path << " -> " << cntName << endmsg << os.str() << endmsg;
      status = saveDescription( path, cntName, os.str(), "", pObj->clID() );
      if ( status.isSuccess() ) {
        status = m_dbMgr->commitOutput( path, true );
        if ( status.isSuccess() ) {
          string        spar[] = { path, cntName };
          unsigned long ipar[] = { (unsigned long)con, ~0x0u };
          status               = m_dbMgr->createAddress( repSvcType(), pObj->clID(), spar, ipar, pAddr );
          if ( status.isSuccess() ) {
            RootAddress* rpA = dynamic_cast<RootAddress*>( pAddr );
            if ( rpA ) {
              ( (unsigned long*)rpA->ipar() )[0] = (unsigned long)con;
              rpA->section                       = tree;
            } else {
              log() << MSG::ERROR << "cannot dynamic cast to RootAddress" << endmsg;
            }

          } else {
            pAddr->release();
            pAddr = nullptr;
          }
        }
      }
      return status;
    }
  }
  return S_FAIL;
}

// Resolve the references of the converted object.
StatusCode RootNTupleCnv::fillRepRefs( IOpaqueAddress* pAddr, DataObject* pObj ) {
  typedef INTuple::ItemContainer Cont;
  INTuple*                       tupl = dynamic_cast<INTuple*>( pObj );
  IRegistry*                     pReg = pObj->registry();
  RootAddress*                   rpA  = dynamic_cast<RootAddress*>( pAddr );
  if ( tupl && pReg && rpA ) {
    string              cntName = containerName( pReg );
    unsigned long*      ipar    = const_cast<unsigned long*>( pAddr->ipar() );
    RootDataConnection* con     = reinterpret_cast<RootDataConnection*>( rpA->ipar()[0] );
    if ( con ) {
      TTree* tree = rpA->section;
      if ( tree ) {
        Cont&            it = tupl->items();
        size_t           k, n = it.size();
        vector<RootRef*> paddr( n );
        vector<RootRef>  addr( n );
        for ( k = 0; k < n; ++k ) {
          IOpaqueAddress*  pA = nullptr;
          Cont::value_type j  = it[k];
          switch ( j->type() ) {
          case DataTypeInfo::OBJECT_ADDR:
            pA       = ( *(IOpaqueAddress**)j->buffer() );
            paddr[k] = &addr[k];
            addr[k].reset();
            if ( pA ) {
              con->makeRef( *pA->registry(), addr[k] );
              addr[k].entry = pA->ipar()[1];
            }
            tree->SetBranchAddress( j->name().c_str(), &paddr[k] );
            break;
          default:
            break;
          }
        }
        int nb = tree->Fill();
        if ( nb > 1 ) ++ipar[1];
        for ( k = 0; k < n; ++k ) it[k]->reset();
        return nb > 1 ? StatusCode::SUCCESS : StatusCode::FAILURE;
      }
      return makeError( "fillRepRefs> Failed to access data tree:" + cntName );
    }
    return makeError( "fillRepRefs> Failed to access data source!" );
  }
  return makeError( "fillRepRefs> Invalid Tuple reference." );
}

#ifdef __POOL_COMPATIBILITY
#  include <RootCnv/PoolClasses.h>

// Compatibility code to access ETCs, which were written using POOL

namespace {
  // Blob I/O helper class
  class IOBuffer : public StreamBuffer {
  public:
    UCharDbArray d;
    IOBuffer() = default;
    virtual ~IOBuffer() {
      m_pointer = 0;
      m_length  = 0;
      m_buffer  = nullptr;
    }
    void start() {
      m_pointer = 0;
      m_buffer  = (char*)d.m_buffer;
      m_length  = d.m_size;
    }
  };
} // namespace

// Helper to read
template <class T>
static inline int load( int blob, IOBuffer& s, void* buff ) {
  if ( blob ) {
    int len;
    s >> len;
    s.swapFromBuffer( buff, len * sizeof( T ) );
  }
  return 0;
}

// Helper to read specialized for strings
template <>
inline int load<string>( int blob, IOBuffer& s, void* ptr ) {
  if ( blob ) {
    string* str = (string*)ptr;
    s >> ( *str );
  }
  return 0;
}

// Update the transient object: NTuples end here when reading records
StatusCode RootNTupleCnv::i__updateObjPool( RootAddress* rpA, INTuple* tupl, TTree* tree, RootDataConnection* con ) {
  typedef INTuple::ItemContainer Cont;
  const string*                  par  = rpA->par();
  unsigned long*                 ipar = const_cast<unsigned long*>( rpA->ipar() );
  ++ipar[1];
  if ( Long64_t( ipar[1] ) <= tree->GetEntries() ) {
    Cont&                    it = tupl->items();
    size_t                   k, n = it.size();
    vector<PoolDbTokenWrap*> paddr( n );
    vector<PoolDbTokenWrap>  addr( n );
    vector<int>              blob_items( n, 0 );
    for ( k = 0; k < n; ++k ) {
      Cont::value_type j = it[k];
      switch ( j->type() ) {
      case DataTypeInfo::OBJECT_ADDR:
        paddr[k] = &addr[k];
        tree->SetBranchAddress( j->name().c_str(), &paddr[k] );
        break;
      default:
        if ( nullptr == tree->GetBranch( j->name().c_str() ) ) blob_items[k] = 1;
        break;
      }
    }
    ULong64_t         last = (ULong64_t)tree->GetEntries();
    ISelectStatement* sel  = tupl->selector();
    if ( sel ) {
      string criteria = ( sel && ( sel->type() & ISelectStatement::STRING ) ) ? sel->criteria() : string( "" );
      if ( !( criteria.length() == 0 || criteria == "*" ) ) {
        if ( rpA->select == nullptr ) {
          log() << MSG::DEBUG << "Selection criteria: " << criteria << "  " << ipar[1] << endmsg;
          rpA->select = new TTreeFormula( tree->GetName(), criteria.c_str(), tree );
        }
        rpA->select->SetTree( tree );
        // loop on all selected entries
        for ( ; ipar[1] < last; ++ipar[1] ) {
          tree->LoadTree( ipar[1] );
          rpA->select->GetNdata();
          if ( fabs( rpA->select->EvalInstance( 0 ) ) > std::numeric_limits<double>::epsilon() ) { break; }
          log() << MSG::DEBUG << par[0] << "/" << par[1] << " SKIP Entry: " << ipar[1] << endmsg;
        }
      }
    }
    if ( ipar[1] < last ) {
      IOBuffer      blob;
      UCharDbArray* pblob = &blob.d;
      tree->GetBranch( "BlobData" )->SetAddress( &pblob );
      if ( tree->GetEntry( ipar[1] ) > 1 ) {
        int sc = 0;
        blob.start();
        for ( k = 0; k < n; ++k ) {
          Cont::value_type j   = it[k];
          char*            buf = (char*)j->buffer();
          switch ( j->type() ) {
          case DataTypeInfo::OBJECT_ADDR: {
            RootRef         r  = con->tool()->poolRef( addr[k].token.m_oid.first );
            GenericAddress* pA = ( *(GenericAddress**)buf );
            if ( pA ) { // Fill only if item is connected!
              string* spar = (string*)pA->par();
              ipar         = (unsigned long*)pA->ipar();
              spar[0]      = con->getDb( r.dbase );
              spar[1]      = con->getCont( r.container );
              spar[2]      = con->getLink( r.link );
              ipar[0]      = 0;
              ipar[1]      = addr[k].token.m_oid.second;
              if ( r.svc == POOL_ROOT_StorageType || r.svc == POOL_ROOTKEY_StorageType ||
                   r.svc == POOL_ROOTTREE_StorageType ) {
                r.svc = ROOT_StorageType;
              }
              pA->setClID( r.clid );
              pA->setSvcType( r.svc );
              break;
            }
            sc = 11;
            break;
          }
          case DataTypeInfo::UCHAR:
            sc = load<unsigned char>( blob_items[k], blob, buf );
            break;
          case DataTypeInfo::USHORT:
            sc = load<unsigned short>( blob_items[k], blob, buf );
            break;
          case DataTypeInfo::UINT:
            sc = load<unsigned int>( blob_items[k], blob, buf );
            break;
          case DataTypeInfo::ULONG:
            sc = load<unsigned long>( blob_items[k], blob, buf );
            break;
          case DataTypeInfo::CHAR:
            sc = load<char>( blob_items[k], blob, buf );
            break;
          case DataTypeInfo::SHORT:
            sc = load<short>( blob_items[k], blob, buf );
            break;
          case DataTypeInfo::INT:
            sc = load<int>( blob_items[k], blob, buf );
            break;
          case DataTypeInfo::LONG:
            sc = load<long>( blob_items[k], blob, buf );
            break;
          case DataTypeInfo::BOOL:
            sc = load<bool>( blob_items[k], blob, buf );
            break;
          case DataTypeInfo::FLOAT:
            sc = load<float>( blob_items[k], blob, buf );
            break;
          case DataTypeInfo::DOUBLE:
            sc = load<double>( blob_items[k], blob, buf );
            break;
          case DataTypeInfo::STRING:
            sc = load<string>( blob_items[k], blob, buf );
            break;
          case DataTypeInfo::NTCHAR:
            sc = load<char*>( blob_items[k], blob, buf );
            break;
          case DataTypeInfo::POINTER:
            sc = 0;
            break;
          case DataTypeInfo::UNKNOWN:
            break;
          default:
            break;
          }
          if ( sc ) {
            log() << MSG::DEBUG;
            switch ( sc ) {
            case 10:
              log() << "CANNOT Set Ntuple token: dynamic_cast<GenericAddress*> is NULL";
              break;
            case 11:
              log() << "CANNOT Set Ntuple token: invalid address buffer";
              break;
            }
            log() << endmsg;
          }
        }
        return StatusCode::SUCCESS;
      }
      log() << MSG::ERROR << "Failed to read data from NTuple tree." << endmsg;
      return StatusCode::FAILURE;
    }
    log() << MSG::INFO << "End of input Ntuple." << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::FAILURE;
}
#endif
