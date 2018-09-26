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
#define ROOTHISTCNV_RCWNTUPLECNV_CPP

// Include files
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/NTuple.h"

// Compiler include files
#include <cstdio>
#include <cstring>
#include <list>
#include <utility>
#include <vector>

#include "RCWNTupleCnv.h"

#include "TLeafD.h"
#include "TLeafF.h"
#include "TLeafI.h"
#include "TTree.h"

namespace {
  template <typename T>
  size_t saveItem( char* target, const NTuple::_Data<T>& src ) {
    static_assert( std::is_trivially_copyable_v<T>, "T must be trivally copyable" );
    std::memcpy( target, src.buffer(), sizeof( T ) * src.length() );
    return sizeof( T ) * src.length();
  }

  template <typename T>
  size_t loadItem( const char* src, NTuple::_Data<T>& target ) {
    static_assert( std::is_trivially_copyable_v<T>, "T must be trivally copyable" );
    std::memcpy( const_cast<void*>( target.buffer() ), src, sizeof( T ) * target.length() );
    return sizeof( T ) * target.length();
  }

  template <typename POD>
  decltype( auto ) downcast_item( const INTupleItem& i ) {
    return dynamic_cast<const NTuple::_Data<POD>&>( i );
  }
  template <typename POD>
  decltype( auto ) downcast_item( INTupleItem& i ) {
    return dynamic_cast<NTuple::_Data<POD>&>( i );
  }
  template <typename POD, typename T>
  void downcast_item( T&& ) = delete;

  template <typename Item, typename F>
  decltype( auto ) visit( Item& i, F&& f ) {
    switch ( i.type() ) {
    case DataTypeInfo::INT:
      return f( downcast_item<int>( i ) );
    case DataTypeInfo::CHAR:
      return f( downcast_item<char>( i ) );
    case DataTypeInfo::SHORT:
      return f( downcast_item<short>( i ) );
    case DataTypeInfo::LONG:
      return f( downcast_item<long>( i ) );
    case DataTypeInfo::LONGLONG:
      return f( downcast_item<long long>( i ) );
    case DataTypeInfo::UCHAR:
      return f( downcast_item<unsigned char>( i ) );
    case DataTypeInfo::USHORT:
      return f( downcast_item<unsigned short>( i ) );
    case DataTypeInfo::UINT:
      return f( downcast_item<unsigned int>( i ) );
    case DataTypeInfo::ULONG:
      return f( downcast_item<unsigned long>( i ) );
    case DataTypeInfo::ULONGLONG:
      return f( downcast_item<unsigned long long>( i ) );
    case DataTypeInfo::DOUBLE:
      return f( downcast_item<double>( i ) );
    case DataTypeInfo::FLOAT:
      return f( downcast_item<float>( i ) );
    case DataTypeInfo::BOOL:
      return f( downcast_item<bool>( i ) );
    }
    throw std::runtime_error( "RCWNTupleCnv::visit: unknown INTupleItem::type()" );
  }

  //-----------------------------------------------------------------------------
  template <class T>
  void analyzeItem( std::string typ, const NTuple::_Data<T>* it, std::string& desc, std::string& block_name,
                    std::string& var_name, long& lowerRange, long& upperRange, long& size )
  //-----------------------------------------------------------------------------
  {

    RootHistCnv::parseName( it->name(), block_name, var_name );

    // long item_size = (sizeof(T) < 4) ? 4 : sizeof(T);
    long item_size = sizeof( T );
    long dimension = it->length();
    long ndim      = it->ndim() - 1;
    desc += var_name;
    if ( it->hasIndex() || it->length() > 1 ) { desc += '['; }
    if ( it->hasIndex() ) {
      std::string ind_blk, ind_var;
      RootHistCnv::parseName( it->index(), ind_blk, ind_var );
      if ( ind_blk != block_name ) {
        std::cerr << "ERROR: Index for CWNT variable " << ind_var << " is in a different block: " << ind_blk
                  << std::endl;
      }
      desc += ind_var;
    } else if ( it->dim( ndim ) > 1 ) {
      desc += std::to_string( it->dim( ndim ) );
    }

    for ( int i = ndim - 1; i >= 0; i-- ) {
      desc += "][";
      desc += std::to_string( it->dim( i ) );
    }
    if ( it->hasIndex() || it->length() > 1 ) { desc += ']'; }

    if ( it->range().lower() != it->range().min() && it->range().upper() != it->range().max() ) {
      lowerRange = it->range().lower();
      upperRange = it->range().upper();
    } else {
      lowerRange = 0;
      upperRange = -1;
    }
    desc += typ;
    size += item_size * dimension;
  }
} // namespace

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::book( const std::string& desc, INTuple* nt, TTree*& rtree )
//-----------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), "RCWNTupleCnv" );
  rtree = new TTree( desc.c_str(), nt->title().c_str() );
  log << MSG::VERBOSE << "created tree id: " << rtree->GetName() << "  title: " << nt->title() << " desc: " << desc
      << endmsg;

  // Loop over the items

  std::string              block_name, var_name;
  long                     lowerRange, upperRange;
  long                     size = 0;
  long                     cursize, oldsize = 0;
  std::vector<std::string> item_fullname;
  //    std::vector<long> item_size,item_size2;
  std::vector<long>                                item_buf_pos, item_buf_len, item_buf_end;
  std::vector<long>                                item_range_lower, item_range_upper;
  std::vector<std::pair<std::string, std::string>> item_name;

  for ( const auto& i : nt->items() ) {
    std::string item;

    visit( *i, [&]( const auto& data ) {
      analyzeItem( this->rootVarType( data.type() ), &data, item, block_name, var_name, lowerRange, upperRange, size );
    } );

    item_name.emplace_back( block_name, item );
    cursize = size - oldsize;

    log << MSG::VERBOSE << "item: " << item << " type " << i->type() << " blk: " << block_name << " var: " << var_name
        << " rng: " << lowerRange << " " << upperRange << " sz: " << size << " " << cursize
        << " buf_pos: " << size - cursize << endmsg;

    item_fullname.push_back( var_name );
    item_buf_pos.push_back( size - cursize );
    item_buf_len.push_back( cursize );
    item_buf_end.push_back( size );
    item_range_lower.push_back( lowerRange );
    item_range_upper.push_back( upperRange );

    oldsize = size;
  }

  // Make a new buffer, and tell the ntuple where it is
  char* buff = nt->setBuffer( new char[size] );

  log << MSG::VERBOSE << "Created buffer size: " << size << " at " << (void*)buff << endmsg;

  // Zero out the buffer to make ROOT happy
  std::fill_n( buff, size, 0 );

  char* buf_pos = buff;

  auto end = item_name.cend();

  // Loop over items, creating a new branch for each one;
  unsigned int i_item = 0;
  for ( auto itr = item_name.cbegin(); itr != end; ++itr, ++i_item ) {

    buf_pos = buff + item_buf_pos[i_item];

    //      log << MSG::WARNING << "adding TBranch " << i_item << "  "
    //  	<< item_fullname[i_item]
    //    	<< "  format: " << itr->second.c_str() << "  at "
    //    	<< (void*) buf_pos << " (" << (void*) buff << "+"
    //  	<< (void*)item_buf_pos[i_item] << ")"
    //  	<< endmsg;

#if ROOT_VERSION_CODE >= ROOT_VERSION( 5, 15, 0 )
    auto br = new TBranch( rtree,
#else
    TBranch* br = new TBranch(
#endif
                           item_fullname[i_item].c_str(), buf_pos, itr->second.c_str() );

    if ( itr->first != "AUTO_BLK" ) {
      std::string title = itr->first;
      title             = itr->first + "::" + br->GetTitle();
      br->SetTitle( title.c_str() );
    }

    log << MSG::DEBUG << "adding TBranch  " << br->GetTitle() << "  at " << (void*)buf_pos << endmsg;

    // for index items with a limited range. Must be a TLeafI!
    if ( item_range_lower[i_item] < item_range_upper[i_item] ) {
      //        log << MSG::VERBOSE << "\"" << item_fullname[i_item]
      //  	  << "\" is range limited " << item_range_lower[i_item] << "  "
      //  	  << item_range_upper[i_item] << endmsg;
      TLeafI*  index = nullptr;
      TObject* tobj  = br->GetListOfLeaves()->FindObject( item_fullname[i_item].c_str() );
      if ( tobj->IsA()->InheritsFrom( "TLeafI" ) ) {
        index = dynamic_cast<TLeafI*>( tobj );

        if ( index ) {
          index->SetMaximum( item_range_upper[i_item] );
          // FIXME -- add for next version of ROOT
          // index->SetMinimum( item_range_lower[i_item] );
        } else {
          log << MSG::ERROR << "Could dynamic cast to TLeafI: " << item_fullname[i_item] << endmsg;
        }
      }
    }

    rtree->GetListOfBranches()->Add( br );
  }

  log << MSG::INFO << "Booked TTree with ID: " << desc << " \"" << nt->title() << "\" in directory " << getDirectory()
      << endmsg;

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::writeData( TTree* rtree, INTuple* nt )
//-----------------------------------------------------------------------------
{
  // Fill the tree;
  const auto& items = nt->items();
  std::accumulate( begin( items ), end( items ), nt->buffer(), []( char* dest, const INTupleItem* i ) {
    return dest + visit( *i, [dest]( const auto& item ) { return saveItem( dest, item ); } );
  } );

  rtree->Fill();
  nt->reset();
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::readData( TTree* rtree, INTuple* ntup, long ievt )
//-----------------------------------------------------------------------------
{
  if ( ievt >= rtree->GetEntries() ) {
    MsgStream log( msgSvc(), "RCWNTupleCnv::readData" );
    log << MSG::ERROR << "no more entries in tree to read. max: " << rtree->GetEntries() << "  current: " << ievt
        << endmsg;
    return StatusCode::FAILURE;
  }

  rtree->GetEvent( ievt );
  ievt++;

  // copy data from ntup->buffer() to ntup->items()->buffer()
  auto& items = ntup->items();
  std::accumulate( begin( items ), end( items ), const_cast<const char*>( ntup->buffer() ),
                   []( const char* src, INTupleItem* i ) {
                     return src + visit( *i, [src]( auto& item ) { return loadItem( src, item ); } );
                   } );

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::load( TTree* tree, INTuple*& refpObject )
//-----------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), "RCWNTupleCnv::load" );

  StatusCode status;

  NTuple::Tuple* pObj = nullptr;

  std::string title = tree->GetTitle();
  log << MSG::VERBOSE << "loading CWNT " << title << " at: " << tree << endmsg;

  status        = m_ntupleSvc->create( CLID_ColumnWiseTuple, title, pObj );
  INTuple* ntup = dynamic_cast<INTuple*>( pObj );
  if ( !ntup ) { log << MSG::ERROR << "cannot dynamic cast to INTuple" << endmsg; }

  INTupleItem* item = nullptr;

  std::string itemName, indexName, item_type, itemTitle, blockName;
  // long numEnt, numVar;
  long                                size, totsize = 0;
  std::vector<std::pair<TLeaf*, int>> itemList;

  // numEnt = (int)tree->GetEntries();
  // numVar = tree->GetNbranches();

  // loop over all branches (==leaves)
  TObjArray* lbr = tree->GetListOfBranches();
  TIter      bitr( lbr );
  while ( TObject* tobjb = bitr() ) {

    TBranch* br = (TBranch*)tobjb;
    itemTitle   = br->GetTitle();

    int ipos = itemTitle.find( "::" );
    if ( ipos >= 0 ) {
      blockName = itemTitle.substr( 0, ipos );
    } else {
      blockName = "";
    }

    TObjArray* lf = br->GetListOfLeaves();

    TIter litr( lf );
    while ( TObject* tobj = litr() ) {

      bool hasRange   = false;
      int  indexRange = 0;
      int  itemSize;
      item = nullptr;

      //      TLeaf* tl = (TLeaf*)tobj;
      TLeaf* tl = dynamic_cast<TLeaf*>( tobj );
      if ( tl ) {
        itemName = tl->GetName();
      } else {
        log << MSG::ERROR << "cannot dynamic cast to TLeaf" << endmsg;
      }

      //	char* buf_pos = (char*)tl->GetValuePointer();
      //  	cout << " " << itemName << "  " << blockName << "  "
      //  	     << (void*)buf_pos;

      if ( blockName != "" ) {
        log << MSG::DEBUG << "loading NTuple item " << blockName << "/" << itemName;
      } else {
        log << MSG::DEBUG << "loading NTuple item " << itemName;
      }

      int    arraySize{0};
      TLeaf* indexLeaf = ( tl ? tl->GetLeafCounter( arraySize ) : nullptr );

      if ( arraySize == 0 ) { log << MSG::ERROR << "TLeaf counter size = 0. This should not happen!" << endmsg; }

      if ( indexLeaf ) {
        // index Arrays and Matrices

        indexName = indexLeaf->GetName();
        //	  indexRange = tl->GetNdata();
        indexRange = indexLeaf->GetMaximum();
        itemSize   = indexRange * tl->GetLenType() * arraySize;

        log << "[" << indexName;

        // Just for Matrices
        if ( arraySize != 1 ) { log << "][" << arraySize; }
        log << "]";

      } else {
        itemSize = tl->GetLenType() * arraySize;

        indexName = "";

        if ( arraySize == 1 ) {
          // Simple items
        } else {
          // Arrays of constant size
          log << "[" << arraySize << "]";
        }
      }

      log << endmsg;

      //  	cout << "  index: " << indexName <<  endl;

      //	size = tl->GetNdata() * tl->GetLenType();
      size = itemSize;
      totsize += size;

      hasRange = tl->IsRange();

      itemList.emplace_back( tl, itemSize );

      // Integer
      if ( tobj->IsA()->InheritsFrom( "TLeafI" ) ) {

        TLeafI* tli = dynamic_cast<TLeafI*>( tobj );
        if ( tli ) {
          if ( tli->IsUnsigned() ) {
            unsigned long min = 0, max = 0;
            if ( hasRange ) {
              min = tli->GetMinimum();
              max = tli->GetMaximum();
            }

            item = createNTupleItem( itemName, blockName, indexName, indexRange, arraySize, min, max, ntup );
          } else {
            long min = 0, max = 0;
            if ( hasRange ) {
              min = tli->GetMinimum();
              max = tli->GetMaximum();
            }

            item = createNTupleItem( itemName, blockName, indexName, indexRange, arraySize, min, max, ntup );
          }
        } else {
          log << MSG::ERROR << "cannot dynamic cast to TLeafI" << endmsg;
        }

        // Float
      } else if ( tobj->IsA()->InheritsFrom( "TLeafF" ) ) {
        float min = 0., max = 0.;

        TLeafF* tlf = dynamic_cast<TLeafF*>( tobj );
        if ( tlf ) {
          if ( hasRange ) {
            min = float( tlf->GetMinimum() );
            max = float( tlf->GetMaximum() );
          }
        } else {
          log << MSG::ERROR << "cannot dynamic cast to TLeafF" << endmsg;
        }

        item = createNTupleItem( itemName, blockName, indexName, indexRange, arraySize, min, max, ntup );

        // Double
      } else if ( tobj->IsA()->InheritsFrom( "TLeafD" ) ) {
        double min = 0., max = 0.;

        TLeafD* tld = dynamic_cast<TLeafD*>( tobj );
        if ( tld ) {
          if ( hasRange ) {
            min = tld->GetMinimum();
            max = tld->GetMaximum();
          }
        } else {
          log << MSG::ERROR << "cannot dynamic cast to TLeafD" << endmsg;
        }

        item = createNTupleItem( itemName, blockName, indexName, indexRange, arraySize, min, max, ntup );

      } else {
        log << MSG::ERROR << "Uknown data type" << endmsg;
      }

      if ( item ) {
        ntup->add( item ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
      } else {
        log << MSG::ERROR << "Unable to create ntuple item \"" << itemName << "\"" << endmsg;
      }

    } // end litr
  }   // end bitr

  log << MSG::DEBUG << "Total buffer size of NTuple: " << totsize << " Bytes." << endmsg;

  char* buf    = ntup->setBuffer( new char[totsize] );
  char* bufpos = buf;

  int ts = 0;
  for ( const auto& iitr : itemList ) {
    TLeaf* leaf  = iitr.first;
    int    isize = iitr.second;

    log << MSG::VERBOSE << "setting TBranch " << leaf->GetBranch()->GetName() << " buffer at " << (void*)bufpos
        << endmsg;

    leaf->GetBranch()->SetAddress( (void*)bufpos );

    //        //testing
    //        if (leaf->IsA()->InheritsFrom("TLeafI")) {
    //  	for (int ievt=0; ievt<5; ievt++) {
    //  	  leaf->GetBranch()->GetEvent(ievt);
    //  	  int *idat = (int*)bufpos;
    //  	  log << MSG::WARNING << leaf->GetName() << ": " << ievt << "   "
    //  	      << *idat << endmsg;

    //  	}
    //        }

    ts += isize;

    bufpos += isize;
  }

  if ( totsize != ts ) { log << MSG::ERROR << "buffer size mismatch: " << ts << "  " << totsize << endmsg; }

  refpObject = ntup;

  return StatusCode::SUCCESS;
}

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_CONVERTER( RootHistCnv::RCWNTupleCnv )
