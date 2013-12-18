#define ROOTHISTCNV_RCWNTUPLECNV_CPP

#define ALLOW_ALL_TYPES

// Include files
#include "GaudiKernel/xtoa.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/NTuple.h"

// Compiler include files
#include <cstdio>
#include <utility> /* std::pair */
#include <list>
#include <vector>

#include "RCWNTupleCnv.h"

#include "TTree.h"
#include "TLeafI.h"
#include "TLeafF.h"
#include "TLeafD.h"

#ifdef __ICC
// disable icc remark #1572: floating-point equality and inequality comparisons are unreliable
//     they are intended
#pragma warning(disable:1572)
#endif

//-----------------------------------------------------------------------------
template <class T> void analyzeItem(std::string typ,
                                    const NTuple::_Data<T>* it,
                                    std::string& desc,
                                    std::string& block_name,
                                    std::string& var_name,
                                    long& lowerRange,
                                    long& upperRange,
                                    long& size)
//-----------------------------------------------------------------------------
{

  std::string full_name = it->name();
  RootHistCnv::parseName(full_name,block_name,var_name);

  //long item_size = (sizeof(T) < 4) ? 4 : sizeof(T);
  long item_size =  sizeof(T);
  long dimension = it->length();
  long ndim = it->ndim()-1;
  std::ostringstream text;
  text << var_name;
  if ( it->hasIndex() || it->length() > 1 )   {
    text << '[';
  }
  if ( it->hasIndex() )
  {
    std::string ind_blk, ind_var;
    std::string ind = it->index();
    RootHistCnv::parseName(ind,ind_blk,ind_var);
    if (ind_blk != block_name) {
      std::cerr << "ERROR: Index for CWNT variable " << ind_var
                << " is in a different block: " << ind_blk << std::endl;
    }
    text << ind_var;
  }
  else if ( it->dim(ndim) > 1 )   {
    text << it->dim(ndim);
  }

  for ( int i = ndim-1; i>=0; i-- ){
    text << "][" << it->dim(i);
  }
  if ( it->hasIndex() || it->length() > 1 )   {
    text << ']';
  }

  if (it->range().lower() != it->range().min() &&
      it->range().upper() != it->range().max() ) {
    lowerRange = (long) it->range().lower();
    upperRange = (long) it->range().upper();
  } else {
    lowerRange = 0;
    upperRange = -1;
  }
  text << typ;

  desc += text.str();
  size += item_size * dimension;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::book(const std::string& desc,
                                           INTuple* nt,
                                           TTree*& rtree)
//-----------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), "RCWNTupleCnv");
  rtree = new TTree(desc.c_str(), nt->title().c_str());
  log << MSG::VERBOSE << "created tree id: " << rtree->GetName()
      << "  title: "<< nt->title() << " desc: " << desc << endmsg;

  // Loop over the items

  std::string block_name,var_name;
  long lowerRange, upperRange;
  long size = 0;
  long cursize, oldsize = 0;
  std::vector<std::string> item_fullname;
  //    std::vector<long> item_size,item_size2;
  std::vector<long> item_buf_pos, item_buf_len, item_buf_end;
  std::vector<long> item_range_lower, item_range_upper;
  std::vector<std::pair<std::string,std::string> > item_name;

  const INTuple::ItemContainer& cols = nt->items();
  for (INTuple::ItemContainer::const_iterator i = cols.begin();
       i != cols.end(); ++i ) {
    std::string item = "";

    switch( (*i)->type() )   {
    case DataTypeInfo::INT:               // int
      analyzeItem(rootVarType( (*i)->type() ),
		  dynamic_cast<const NTuple::_Data<int>*>(*i),item,
		  block_name,var_name,lowerRange,upperRange,size);
      break;
    case DataTypeInfo::CHAR:              // char
      analyzeItem(rootVarType( (*i)->type() ),
		  dynamic_cast<const NTuple::_Data<char>*>(*i),
		  item, block_name,var_name,lowerRange,upperRange,size);
      break;
    case DataTypeInfo::SHORT:             // short
      analyzeItem(rootVarType( (*i)->type() ),
		  dynamic_cast<const NTuple::_Data<short>*>(*i),item,
		  block_name,var_name,lowerRange,upperRange,size);
      break;
    case DataTypeInfo::LONG:              // long
      analyzeItem(rootVarType( (*i)->type() ),
		  dynamic_cast<const NTuple::_Data<long>*>(*i),item,
		  block_name,var_name,lowerRange,upperRange,size);
      break;
    case DataTypeInfo::LONGLONG:          // long long
      analyzeItem(rootVarType( (*i)->type() ),
		  dynamic_cast<const NTuple::_Data<long long>*>(*i),item,
		  block_name,var_name,lowerRange,upperRange,size);
      break;
    case DataTypeInfo::UCHAR:             // unsigned char
      analyzeItem(rootVarType( (*i)->type() ),
		  dynamic_cast<const NTuple::_Data<unsigned char>*>(*i),
		  item, block_name,var_name,lowerRange,upperRange,size);
      break;
    case DataTypeInfo::USHORT:            // unsigned short
      analyzeItem(rootVarType( (*i)->type() ),
		  dynamic_cast<const NTuple::_Data<unsigned short>*>(*i),
		  item, block_name,var_name,lowerRange,upperRange,size);
      break;
    case DataTypeInfo::UINT:              // unsigned int
      analyzeItem(rootVarType( (*i)->type() ),
		  dynamic_cast<const NTuple::_Data<unsigned int>*>(*i),
		  item, block_name,var_name,lowerRange,upperRange,size);
      break;
    case DataTypeInfo::ULONG:             // unsigned long
      analyzeItem(rootVarType( (*i)->type() ),
		  dynamic_cast<const NTuple::_Data<unsigned long>*>(*i),
		  item, block_name,var_name,lowerRange,upperRange,size);
      break;
    case DataTypeInfo::ULONGLONG:         // unsigned long long
      analyzeItem(rootVarType( (*i)->type() ),
		  dynamic_cast<const NTuple::_Data<unsigned long long>*>(*i),
		  item, block_name,var_name,lowerRange,upperRange,size);
      break;
    case DataTypeInfo::DOUBLE:            // double
      analyzeItem(rootVarType( (*i)->type() ),
		  dynamic_cast<const NTuple::_Data<double>*>(*i),item,
		  block_name,var_name,lowerRange,upperRange,size);
      break;
    case DataTypeInfo::FLOAT:             // float
      analyzeItem(rootVarType( (*i)->type() ),
		  dynamic_cast<const NTuple::_Data<float>*>(*i),item,
		  block_name,var_name,lowerRange,upperRange,size);
      break;
    case DataTypeInfo::BOOL:               // bool
      analyzeItem(rootVarType( (*i)->type() ),
		  dynamic_cast<const NTuple::_Data<bool>*>(*i),item,
		  block_name,var_name,lowerRange,upperRange,size);
      break;
    default:
      break;
    }

    item_name.push_back(std::pair<std::string,std::string>(block_name,item));
    cursize = size - oldsize;

    log << MSG::VERBOSE << "item: " << item << " type " << (*i)->type()
        << " blk: " << block_name
        << " var: " << var_name << " rng: " << lowerRange << " "
        << upperRange << " sz: " << size << " " << cursize
        << " buf_pos: " << size-cursize << endmsg;

    item_fullname.push_back(var_name);
    item_buf_pos.push_back(size-cursize);
    item_buf_len.push_back(cursize);
    item_buf_end.push_back(size);
    item_range_lower.push_back(lowerRange);
    item_range_upper.push_back(upperRange);

    oldsize = size;
  }

  char* buff = new char[size];
  log << MSG::VERBOSE << "Created buffer size: " << size << " at "
      << (void*)buff << endmsg;

  // Zero out the buffer to make ROOT happy
  memset (buff,0,size);

  // Tell the ntuple where it is
  nt->setBuffer(buff);

  char *buf_pos = buff;

  std::vector<std::pair<std::string,std::string> >::const_iterator itr,end;
  end = item_name.end();

  // Loop over items, creating a new branch for each one;
  unsigned int i_item = 0;
  for (itr=item_name.begin(); itr!=end; ++itr, ++i_item) {

    buf_pos = buff + item_buf_pos[i_item];

//      log << MSG::WARNING << "adding TBranch " << i_item << "  "
//  	<< item_fullname[i_item]
//    	<< "  format: " << itr->second.c_str() << "  at "
//    	<< (void*) buf_pos << " (" << (void*) buff << "+"
//  	<< (void*)item_buf_pos[i_item] << ")"
//  	<< endmsg;

#if ROOT_VERSION_CODE >= ROOT_VERSION(5,15,0)
    TBranch *br = new TBranch(rtree,
#else
    TBranch *br = new TBranch(
#endif
                              item_fullname[i_item].c_str(),
                              buf_pos,
                              itr->second.c_str()
                              );

    if (itr->first != "AUTO_BLK") {
      std::string tit = itr->first;
      tit = itr->first + "::" + br->GetTitle();
      br->SetTitle(tit.c_str());
    }

    log << MSG::DEBUG << "adding TBranch  " << br->GetTitle() << "  at "
        << (void*) buf_pos << endmsg;


    // for index items with a limited range. Must be a TLeafI!
    if ( item_range_lower[i_item] < item_range_upper[i_item] ) {
//        log << MSG::VERBOSE << "\"" << item_fullname[i_item]
//  	  << "\" is range limited " << item_range_lower[i_item] << "  "
//  	  << item_range_upper[i_item] << endmsg;
      TLeafI *index = 0;
      TObject *tobj = br->GetListOfLeaves()->FindObject( item_fullname[i_item].c_str() );
      if (tobj->IsA()->InheritsFrom("TLeafI")) {
        index = dynamic_cast<TLeafI*>(tobj);

        if (index != 0) {
          index->SetMaximum( item_range_upper[i_item] );
          // FIXME -- add for next version of ROOT
          // index->SetMinimum( item_range_lower[i_item] );
        } else {
          log << MSG::ERROR << "Could dynamic cast to TLeafI: "
              << item_fullname[i_item] << endmsg;
        }
      }
    }

    rtree->GetListOfBranches()->Add(br);

  }


  log << MSG::INFO << "Booked TTree with ID: " << desc
      << " \"" << nt->title() << "\" in directory "
      << getDirectory() << endmsg;

  return StatusCode::SUCCESS;

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::writeData(TTree* rtree, INTuple* nt)
//-----------------------------------------------------------------------------
{
  // Fill the tree;
  const INTuple::ItemContainer& cols = nt->items();
  char * tar = nt->buffer();
  for (INTuple::ItemContainer::const_iterator i = cols.begin(); i != cols.end(); i++ )   {
    switch( (*i)->type() )   {
    case DataTypeInfo::BOOL:              // bool
      tar += saveItem(tar, (bool*)(*i)->buffer(),   (*i)->length());
      break;
    case DataTypeInfo::CHAR:              // char
      tar += saveItem(tar, (char*)(*i)->buffer(),   (*i)->length());
      break;
    case DataTypeInfo::SHORT:             // short
      tar += saveItem(tar, (short*)(*i)->buffer(),  (*i)->length());
      break;
    case DataTypeInfo::INT:             // int
      tar += saveItem(tar, (int*)(*i)->buffer(),    (*i)->length());
      break;
    case DataTypeInfo::LONG:             // long
      tar += saveItem(tar, (long*)(*i)->buffer(),   (*i)->length());
      break;
    case DataTypeInfo::LONGLONG:         // long long
      tar += saveItem(tar, (long long*)(*i)->buffer(),   (*i)->length());
      break;
    case DataTypeInfo::UCHAR:             // unsigned char
      tar += saveItem(tar, (unsigned char*)(*i)->buffer(),  (*i)->length());
      break;
    case DataTypeInfo::USHORT:            // unsigned short
      tar += saveItem(tar, (unsigned short*)(*i)->buffer(), (*i)->length());
      break;
    case DataTypeInfo::UINT:            // unsigned int
      tar += saveItem(tar, (unsigned int*)(*i)->buffer(),   (*i)->length());
      break;
    case DataTypeInfo::ULONG:            // unsigned long
      tar += saveItem(tar, (unsigned long*)(*i)->buffer(),  (*i)->length());
      break;
    case DataTypeInfo::ULONGLONG:        // unsigned long
      tar += saveItem(tar, (unsigned long long*)(*i)->buffer(),  (*i)->length());
      break;
    case DataTypeInfo::FLOAT:            // float
      tar += saveItem(tar, (float*)(*i)->buffer(), (*i)->length());
      break;
    case DataTypeInfo::DOUBLE:            // double
      tar += saveItem(tar, (double*)(*i)->buffer(), (*i)->length());
      break;
    default:
      break;
    }
  }

  rtree->Fill();
  nt->reset();
  return StatusCode::SUCCESS;
}


//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::readData(TTree* rtree,
                                               INTuple* ntup,
                                               long ievt)
//-----------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), "RCWNTupleCnv::readData");

  if (ievt >= rtree->GetEntries()) {
    log << MSG::ERROR << "no more entries in tree to read. max: "
	<< rtree->GetEntries() << "  current: " << ievt
	<< endmsg;
    return StatusCode::FAILURE;
  }

  rtree->GetEvent(ievt);

  ievt++;

  INTuple::ItemContainer& cols = ntup->items();
  char * src = ntup->buffer();

  // copy data from ntup->buffer() to ntup->items()->buffer()

  for (INTuple::ItemContainer::iterator i = cols.begin(); i != cols.end(); i++ )   {


    switch( (*i)->type() )   {
    case DataTypeInfo::BOOL:              // bool
      src += loadItem(src, (bool*)(*i)->buffer(), (*i)->length());
      break;
    case DataTypeInfo::CHAR:              // char
      src += loadItem(src, (char*)(*i)->buffer(), (*i)->length());
      break;
    case DataTypeInfo::SHORT:             // short
      src += loadItem(src, (short*)(*i)->buffer(), (*i)->length());
      break;
    case DataTypeInfo::INT:             // short
      src += loadItem(src, (int*)(*i)->buffer(), (*i)->length());
      break;
    case DataTypeInfo::LONG:             // long
      src += loadItem(src, (long*)(*i)->buffer(), (*i)->length());
      break;
    case DataTypeInfo::LONGLONG:         // long long
      src += loadItem(src, (long long*)(*i)->buffer(), (*i)->length());
      break;
    case DataTypeInfo::UCHAR:             // unsigned char
      src += loadItem(src, (unsigned char*)(*i)->buffer(), (*i)->length());
      break;
    case DataTypeInfo::USHORT:            // unsigned short
      src += loadItem(src, (unsigned short*)(*i)->buffer(), (*i)->length());
      break;
    case DataTypeInfo::UINT:            // unsigned short
      src += loadItem(src, (unsigned int*)(*i)->buffer(), (*i)->length());
      break;
    case DataTypeInfo::ULONG:            // unsigned long
      src += loadItem(src, (unsigned long*)(*i)->buffer(), (*i)->length());
      break;
    case DataTypeInfo::ULONGLONG:        // unsigned long long
      src += loadItem(src, (unsigned long long*)(*i)->buffer(), (*i)->length());
      break;
    case DataTypeInfo::FLOAT:            // float
      src += loadItem(src, (float*)(*i)->buffer(), (*i)->length());
      break;
    case DataTypeInfo::DOUBLE:            // unsigned short
      src += loadItem(src, (double*)(*i)->buffer(), (*i)->length());
      break;
    default:
      break;
    }

  }

  return StatusCode::SUCCESS;
}


//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::load(TTree* tree, INTuple*& refpObject )
//-----------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), "RCWNTupleCnv::load");

  StatusCode status;

  NTuple::Tuple *pObj = 0;

  std::string title = tree->GetTitle();
  log << MSG::VERBOSE << "loading CWNT " << title << " at: "
	<< tree << endmsg;

  status = m_ntupleSvc->create(CLID_ColumnWiseTuple, title, pObj);
  INTuple* ntup = dynamic_cast<INTuple*>(pObj);

  INTupleItem* item=0;

  std::string itemName, indexName, item_type, itemTitle, blockName;
  // long numEnt, numVar;
  long size, totsize=0;
  std::list< std::pair<TLeaf*,int> > itemList;

  // numEnt = (int)tree->GetEntries();
  // numVar = tree->GetNbranches();

  // loop over all branches (==leaves)
  TObjArray* lbr = tree->GetListOfBranches();
  TIter bitr ( lbr );
  while ( TObject *tobjb = bitr() ) {

    TBranch* br = (TBranch*)tobjb;
    itemTitle = br->GetTitle();

    int ipos = itemTitle.find("::");
    if (ipos >= 0) {
      blockName = itemTitle.substr(0,ipos);
    } else {
      blockName = "";
    }

    TObjArray* lf = br->GetListOfLeaves();

    TIter litr ( lf );
    while ( TObject *tobj = litr() ) {

      bool hasRange=false;
      int indexRange = 0;
      int itemSize;
      item = 0;

      //      TLeaf* tl = (TLeaf*)tobj;
      TLeaf* tl = dynamic_cast<TLeaf*> (tobj);
      itemName = tl->GetName();

      //	char* buf_pos = (char*)tl->GetValuePointer();
      //  	cout << " " << itemName << "  " << blockName << "  "
      //  	     << (void*)buf_pos;


      if (blockName != "") {
        log << MSG::DEBUG << "loading NTuple item " << blockName << "/"
	    << itemName;
      } else {
        log << MSG::DEBUG << "loading NTuple item " << itemName;
      }

      int arraySize;
      TLeaf* indexLeaf = tl->GetLeafCounter(arraySize);

      if (arraySize == 0) {
	log << MSG::ERROR << "TLeaf counter size = 0. This should not happen!"
	    << endmsg;
      }

      if (indexLeaf != 0) {
        //index Arrays and Matrices

        indexName = indexLeaf->GetName();
        //	  indexRange = tl->GetNdata();
        indexRange = indexLeaf->GetMaximum();
        itemSize = indexRange * tl->GetLenType() * arraySize;

        log << "[" << indexName;

	// Just for Matrices
	if (arraySize != 1) {
	  log << "][" << arraySize;
	}
	log << "]";

      } else {
	itemSize = tl->GetLenType() * arraySize;

	indexName = "";

	if (arraySize == 1) {
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

      itemList.push_back(std::pair<TLeaf*,int>(tl,itemSize));


      // Integer
      if (tobj->IsA()->InheritsFrom("TLeafI")) {

        TLeafI *tli = dynamic_cast<TLeafI*>(tobj);
        if (tli->IsUnsigned()) {
          unsigned long min=0, max=0;
          if (hasRange) {
            min = tli->GetMinimum();
            max = tli->GetMaximum();
          }

          item = createNTupleItem(itemName, blockName, indexName, indexRange,
				  arraySize,
                                  min, max, ntup);
        } else {
          long min=0, max=0;
          if (hasRange) {
            min = tli->GetMinimum();
            max = tli->GetMaximum();
          }

          item = createNTupleItem(itemName, blockName, indexName, indexRange,
				  arraySize,
                                  min, max, ntup);
        }

        // Float
      } else if (tobj->IsA()->InheritsFrom("TLeafF")) {
        float min=0., max=0.;

        TLeafF *tlf = dynamic_cast<TLeafF*>(tobj);
        if (hasRange) {
          min = float(tlf->GetMinimum());
          max = float(tlf->GetMaximum());
        }

        item = createNTupleItem(itemName, blockName, indexName, indexRange,
				arraySize,
                                min, max, ntup);

        // Double
      } else if (tobj->IsA()->InheritsFrom("TLeafD")) {
        double min=0., max=0.;

        TLeafD *tld = dynamic_cast<TLeafD*>(tobj);
        if (hasRange) {
          min = tld->GetMinimum();
          max = tld->GetMaximum();
        }

        item = createNTupleItem(itemName, blockName, indexName, indexRange,
				arraySize,
                                min, max, ntup);


      } else {
        log << MSG::ERROR << "Uknown data type" << endmsg;
      }


      if (item) {
        ntup->add(item);
      } else {
        log << MSG::ERROR
	    << "Unable to create ntuple item \""
	    <<  itemName << "\"" << endmsg;
      }

    } // end litr
  }   // end bitr

  log << MSG::DEBUG << "Total buffer size of NTuple: " << totsize
      << " Bytes." << endmsg;

  char* buf = new char[totsize];
  char* bufpos = buf;

  ntup->setBuffer(buf);

  std::list< std::pair<TLeaf*,int> >::const_iterator iitr = itemList.begin();
  int ts = 0;
  for (; iitr!= itemList.end(); ++iitr) {
    TLeaf* leaf = (*iitr).first;
    int isize   = (*iitr).second;

    log << MSG::VERBOSE << "setting TBranch " << leaf->GetBranch()->GetName()
        << " buffer at " << (void*) bufpos << endmsg;

    leaf->GetBranch()->SetAddress((void*)bufpos);

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

  if (totsize != ts ) {
    log << MSG::ERROR << "buffer size mismatch: " << ts << "  " << totsize
        << endmsg;
  }

  refpObject = ntup;

  return StatusCode::SUCCESS;
}

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_NAMESPACE_CONVERTER_FACTORY(RootHistCnv, RCWNTupleCnv)
