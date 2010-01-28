// $Id: PoolDbNTupleCnv.cpp,v 1.16 2008/10/27 16:41:33 marcocle Exp $
//------------------------------------------------------------------------------
//
// Implementation of class :  PoolDbNTupleCnv
//
// Author :                   Markus Frank
//
//------------------------------------------------------------------------------

// FIXME: missing in CORAL
#include <algorithm>

#define ALLOW_ALL_TYPES
// Include files
#include "GaudiPoolDb/IPoolDbMgr.h"
#include "GaudiPoolDb/PoolDbAddress.h"
#include "GaudiPoolDb/PoolDbNTupleCnv.h"
#include "GaudiPoolDb/PoolDbLinkManager.h"
#include "GaudiPoolDb/PoolDbNTupleDescriptor.h"
#include "GaudiPoolDb/PoolDbTupleCallback.h"

#include "GaudiKernel/xtoa.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/SmartRef.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/ISelectStatement.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/CnvFactory.h"

#include "StorageSvc/DbSelect.h"
#include "StorageSvc/DbReflex.h"
#include "StorageSvc/DbColumn.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbObjectCallBack.h"

#define S_OK   StatusCode::SUCCESS
#define S_FAIL StatusCode::FAILURE
#include "Reflex/Builder/ReflexBuilder.h"

#include <memory>

namespace pool  {
  const std::string typeName(const std::type_info& typ);
}

void popCurrentDataObject();
void pushCurrentDataObject(DataObject** pobjAddr);

namespace pool { void genMD5(const std::string& s, void* code); }
using pool::genMD5;

template <class T> static inline
void* save(pool::DbBlob& s, const void* buffer, int len, bool save_len)  {
  const T* buff = (const T*)buffer;
  if ( len > 1 )  {
    s << len;
    s.swapToBuffer(buff, len*sizeof(T));
    return 0;
  }
  else if ( save_len )  {
    s << len;
    return 0;
  }
  return (void*)buff;
}

static void* save(const void* b, PoolDbTokenWrap* lnk)  {
  IOpaqueAddress* pA = (*(IOpaqueAddress**)b);
  pool::Token& tok   = lnk->token;
  pool::Guid guid(pool::Guid::null());
  if ( pA )  {
    const std::string*   spar = pA->par();
    const unsigned long* ipar = pA->ipar();
    guid.Data1 = pA->clID();
    tok.setTechnology(pA->svcType());
    tok.setDb(spar[0]).setCont(spar[1]);
    tok.oid().first  = ipar[0];
    tok.oid().second = ipar[1];
  }
  else {
    tok.setTechnology(0).setDb("").setCont("");
    tok.oid().first = tok.oid().second = pool::INVALID;
  }
  tok.setClassID(guid);
  return lnk;
}

// Helper to read
template <class T> static inline int load(pool::DbBlob& s, void* buff)  {
  int len;
  s >> len;
  s.swapFromBuffer(buff, len*sizeof(T));
  return 0;
}

// Helper to read specialized for strings
template <> inline int load<std::string>(pool::DbBlob& s, void* ptr)   {
  std::string* str = (std::string*)ptr;
  s >> (*str);
  return 0;
}

// Helper to read specialized for opaque addresses
static inline int load(void* ptr,PoolDbTokenWrap* lnk)   {
  IOpaqueAddress* pA = *(IOpaqueAddress**)ptr;
  GenericAddress* pAddr = (GenericAddress*)(pA);
  if ( 0 != pAddr )  {
    pool::Token&    tok = lnk->token;
    std::string*   spar = (std::string*)pAddr->par();
    unsigned long* ipar = (unsigned long*)pAddr->ipar();
    pAddr->setClID(tok.classID().Data1);
    pAddr->setSvcType(tok.technology());
    ipar[0] = tok.oid().first;
    ipar[1] = tok.oid().second;
    spar[0] = tok.dbID();
    spar[1] = tok.contID();
    return 0;
  }
  else if ( pA ) {
    return 10;
	}
  return 11;
}

// Instantiation of a static factory class used by clients to create
// instances of this service
PLUGINSVC_FACTORY_WITH_ID( PoolDbNTupleCnv,
                           ConverterID(POOL_StorageType,CLID_RowWiseTuple),
                           IConverter*(long, CLID, ISvcLocator*) )

PLUGINSVC_FACTORY_WITH_ID( PoolDbNTupleCnv,
                           ConverterID(POOL_StorageType,CLID_ColumnWiseTuple),
                           IConverter*(long, CLID, ISvcLocator*) )

static inline std::istream&
operator>>(std::istream& is, IOpaqueAddress*& /*pObj*/)   {
  long i;
  is >> i;
  return is;
}

template<class TYP> static
StatusCode createItem ( INTuple* tuple, std::istream& is,
                        const std::string& name,
                        const TYP& null)
{
  std::string idxName;
  long len, ndim, dim[4], hasIdx, idxLow, idxLen;
  long dim1 = 1, dim2 = 1;
  INTupleItem* it = 0;
  char c;
  is >> len    >> c
     >> ndim   >> c
     >> hasIdx >> c;
  if ( hasIdx )  {
    std::getline(is, idxName, ';') >> idxLow >> c >> idxLen >> c;
  }
  for ( int i = 0; i < ndim; i++ )
    is >> dim[i] >> c;

  TYP low = null, high = null;
  is >> low >> c >> high >> c;
  is >> c;
  switch( ndim )  {
  case 0:
    it = NTuple::_Item<TYP>::create (tuple, name, typeid(TYP), low, high, null);
    break;
  case 1:
    dim1 = (hasIdx) ? idxLen : dim[0];
    it = NTuple::_Array<TYP>::create (tuple,
                                      name,
                                      typeid(TYP),
                                      idxName,
                                      dim1,
                                      low,
                                      high,
                                      null);
    break;
  case 2:
    dim1 = (hasIdx) ? idxLen : dim[0];
    dim2 = (hasIdx) ? dim[0] : dim[1];
    it = NTuple::_Matrix<TYP>::create ( tuple,
                                        name,
                                        typeid(TYP),
                                        idxName,
                                        dim1,
                                        dim2,
                                        low,
                                        high,
                                        null);
    break;
  default:
    return S_FAIL;
  }
  return tuple->add(it);
}

template <class T> static inline
void putRange(std::ostream& os, NTuple::_Data<T>* it)
{
  const NTuple::Range<T>& x = it->range();
  os << x.lower() << ';' << x.upper() << ';';
}

/// Standard constructor
PoolDbNTupleCnv::PoolDbNTupleCnv(long typ, const CLID& clid, ISvcLocator* svc)
: PoolDbStatCnv(typ, clid, svc)
{
  m_class = 0;
}

PoolDbNTupleCnv::~PoolDbNTupleCnv()  {
}

// Converter overrides: Update the references of an updated transient object.
StatusCode
PoolDbNTupleCnv::createObj(IOpaqueAddress* pAddress, DataObject*& refpObject)
{
  StatusCode status = S_FAIL;
  PoolDbAddress* pdbA = dynamic_cast<PoolDbAddress*>(pAddress);
  if ( 0 != pdbA )  {
    const pool::Token* tok = pdbA->token();
    if ( tok )  {
      pool::DbContainer cntH(tok->technology());
      status = m_dbMgr->connect(tok->dbID(),tok->contID(),cntH);
      if ( status.isSuccess() ) {
        std::string par_val, par_guid, par_typ;
        std::auto_ptr<pool::DbSelect> iter(m_dbMgr->createSelect("*", tok->dbID(), "GaudiStatisticsDescription"));
        if ( iter.get() )  {
          pool::Token* token;
          ROOT::Reflex::Type typ = pool::DbReflex::forTypeName("PoolDbNTupleDescriptor");
          pool::DbObjectCallBack* call = new pool::DbObjectCallBack(typ);
          while( iter->next(token).isSuccess() )  {
            typedef pool::DbObjectHandle<pool::DbObject> ObjH;
            pool::DbContainer& cH = iter->container();
            if ( ObjH::openEx(cH, *token, call, pool::READ).isSuccess() )  {
              PoolDbNTupleDescriptor* ref = (PoolDbNTupleDescriptor*)call->object();
              if ( ref->container == tok->contID() )  {
                par_val  = ref->description;
                par_typ  = ref->optional;
                par_guid = ref->guid;
                token->release();
                call->release();
                call = 0;
                delete ref;
                break;
              }
              delete ref;
            }
            token->release();
          }
          if ( call ) call->release();
        }
        if ( !par_val.empty() && !par_guid.empty() && !par_typ.empty() )
        {
          SmartIF<INTupleSvc> ntupleSvc(dataProvider());
          if ( ntupleSvc.isValid() )  {
            char c;
            CLID clid;
            int siz, typ;
            std::string title;
            NTuple::Tuple* nt = 0;
            std::istringstream is(par_val);
            std::getline(is, title, ';') >> clid >> c >> siz >> c;
            status = ntupleSvc->create(clid, title, nt);
            for ( int j = 0; j < siz && status.isSuccess(); j++ ) {
      	      is >> c;
              std::getline(is, title, ';') >> typ >> c;
              switch ( typ )    {
              case DataTypeInfo::UCHAR:
                status = createItem(nt, is, title, (unsigned char)0);
                break;
              case DataTypeInfo::USHORT:
                status = createItem(nt, is, title, (unsigned short)0);
                break;
              case DataTypeInfo::UINT:
                status = createItem(nt, is, title, (unsigned int)0);
                break;
              case DataTypeInfo::ULONG:
                status = createItem(nt, is, title, (unsigned long)0);
                break;
              case DataTypeInfo::CHAR:
                status = createItem(nt, is, title, char(0));
                break;
              case DataTypeInfo::SHORT:
                status = createItem(nt, is, title, short(0));
                break;
              case DataTypeInfo::INT:
                status = createItem(nt, is, title, int(0));
                break;
              case DataTypeInfo::LONG:
                status = createItem(nt, is, title, long(0));
                break;
              case DataTypeInfo::BOOL:
                status = createItem(nt, is, title, false);
                break;
              case DataTypeInfo::FLOAT:
                status = createItem(nt, is, title, float(0.0));
                break;
              case DataTypeInfo::DOUBLE:
                status = createItem(nt, is, title, double(0.0));
                break;
                /*
              case DataTypeInfo::NTCHAR:
              case DataTypeInfo::LONG_NTCHAR:
                status =
                createItem(nt, is, title, (char*)0);
                break;
              case DataTypeInfo::STRING:
              case DataTypeInfo::LONG_STRING:
                status =
                createItem(nt, is, title, std::string(""));
                break;
                */
              case DataTypeInfo::OBJECT_ADDR:
                status = createItem(nt, is, title, (IOpaqueAddress*)0);
                break;
              case DataTypeInfo::POINTER:
                status = createItem(nt, is, title, (void*)0);
                break;
              case DataTypeInfo::UNKNOWN:
              default:
                status = S_FAIL;
                break;
              }
              if ( !status.isSuccess() )  {
                MsgStream err(msgSvc(),"NTupleCnv");
                err << MSG::FATAL
                    << "Error connecting (Unknown) column:" << j << endmsg
                    << par_val << endmsg;
                return makeError("createObj[NTuple]> Cannot determine column!");
              }
            }
            if ( status.isSuccess() )  {
              PoolDbTupleCallback* hdlr = new PoolDbTupleCallback(nt);
              const pool::DbTypeInfo* typ = pool::DbTypeInfo::fromString(par_typ);
              hdlr->configure(nt, typ, cntH);
              pdbA->setHandler(hdlr);
              refpObject  = nt;
            }
            else {
              refpObject = 0;
              if ( nt ) nt->release();
            }
          }
        }
      }
    }
  }
  return status;
}

// Update the transient object: NTuples end here when reading records
StatusCode
PoolDbNTupleCnv::updateObj(IOpaqueAddress* pAddr, DataObject* pObj)  {
  INTuple*       tupl = dynamic_cast<INTuple*>(pObj);
  PoolDbAddress* pdbA = dynamic_cast<PoolDbAddress*>(pAddr);
  if ( 0 != tupl && 0 != pdbA )  {
    PoolDbTupleCallback* hdlr = dynamic_cast<PoolDbTupleCallback*>(pdbA->handler());
    pool::DbSelect* it = hdlr->iterator();
    if ( 0 == it )  {
      it = hdlr->select(tupl->selector());
    }
    if ( it )  {
      pool::Token* t = 0;
      if ( hdlr->iterator()->next(t).isSuccess() )  {
        std::auto_ptr<pool::Token> next(t);
        // Now reload!
        if ( bindRead(tupl, hdlr).isSuccess() )  {
          // No support for SmartRefs<T> for N-tuples
          //pushCurrentDataObject((DataObject**)&pObj);
          if ( m_dbMgr->read(hdlr, *next.get()).isSuccess() )  {
            if ( readData(tupl, hdlr).isSuccess() )  {
              //popCurrentDataObject();
              return S_OK;
            }
            //popCurrentDataObject();
            return makeError("updateObj> Cannot interprete data.");
          }
          //popCurrentDataObject();
          return makeError("updateObj> Cannot read data.");
        }
        return makeError("updateObj> Cannot bind data.");
      }
      return S_FAIL;
    }
    return makeError("updateObj> Token selection failed!");
  }
  return makeError("updateObj> Invalid Tuple reference.");
}

/// Convert the transient object to the requested representation.
StatusCode
PoolDbNTupleCnv::createRep(DataObject* pObj, IOpaqueAddress*& pAddr)  {
  IRegistry* pRegistry = pObj->registry();
  if ( 0 != pRegistry )  {
    pAddr = pRegistry->address();
    PoolDbAddress* pdbA = dynamic_cast<PoolDbAddress*>(pAddr);
    if ( 0 == pdbA )  {
      const INTuple* nt  = dynamic_cast<const INTuple*>(pObj);
      if ( 0 != nt )  {
        const INTuple::ItemContainer& items = nt->items();
        std::vector<const pool::DbColumn*> cols;
        std::vector<int> item_map(items.size()+1,-1);
        std::ostringstream os;
        int pool_type = 0;
        size_t item_no;
        os << nt->title()<<';'<<pObj->clID()<<';'<<items.size()<< ';';
        for(item_no = 0; item_no < items.size(); ++item_no ) {
          pool::DbColumn* col = 0;
          INTupleItem* it = items[item_no];
          os << '{'
             << it->name()      << ';'
             << it->type()      << ';'
             << it->length()    << ';'
             << it->ndim()      << ';'
             << it->hasIndex()  << ';';
          if ( it->hasIndex() )   {
            os << it->index() << ';';
            INTupleItem* itm = it->indexItem();
            switch( itm->type() )    {
            case DataTypeInfo::UCHAR:
              putRange(os, dynamic_cast<NTuple::_Data<unsigned char>*>(itm));
              break;
            case DataTypeInfo::USHORT:
              putRange(os, dynamic_cast<NTuple::_Data<unsigned short>*>(itm));
              break;
            case DataTypeInfo::UINT:
              putRange(os, dynamic_cast<NTuple::_Data<unsigned int>*>(itm));
              break;
            case DataTypeInfo::ULONG:
              putRange(os, dynamic_cast<NTuple::_Data<unsigned long>*>(itm));
              break;
            case DataTypeInfo::CHAR:
              putRange(os, dynamic_cast<NTuple::_Data<char>*>(itm));
              break;
            case DataTypeInfo::SHORT:
              putRange(os, dynamic_cast<NTuple::_Data<short>*>(itm));
              break;
            case DataTypeInfo::INT:
              putRange(os, dynamic_cast<NTuple::_Data<int>*>(itm));
              break;
            case DataTypeInfo::LONG:
              putRange(os, dynamic_cast<NTuple::_Data<long>*>(itm));
              break;
            default: {
              MsgStream err(msgSvc(), "NTuple:"+pRegistry->name());
              err << MSG::ERROR << "Column " << it->index()
                  << " is not a valid index column!" << endmsg;
              return S_FAIL;
              }
            }
          }
          for ( long k = 0; k < it->ndim(); k++ )  {
            os << it->dim(k) << ';';
          }
          switch(it->type())  {
            case DataTypeInfo::STRING:
              pool_type = pool::DbColumn::STRING;
              os << 0 << ';' << 0 << ';';
              break;
            case DataTypeInfo::NTCHAR:
              pool_type = pool::DbColumn::NTCHAR;
              os << 0 << ';' << 0 << ';';
              break;
            case DataTypeInfo::OBJECT_ADDR:
              if ( it->length() == 1 )  {
                col = new pool::DbColumn(it->name(),
                                         pool::typeName(typeid(PoolDbTokenWrap)),
                                         pool::DbColumn::POINTER,
                                         0);
                item_map[item_no] = cols.size();
                cols.push_back(col);
              }
              os << 0 << ';' << 0 << ';';
              break;
            case DataTypeInfo::POINTER:
              if ( it->length() == 1 )  {
                ROOT::Reflex::Type typ = ROOT::Reflex::Type::ByName(it->typeName());
                col = new pool::DbColumn(it->name(),
                                         typ.Name(ROOT::Reflex::SCOPED),
                                         pool::DbColumn::POINTER,
                                         0);
                item_map[item_no] = cols.size();
                cols.push_back(col);
                os << 0 << ';' << 0 << ';';
              }
              break;
            case DataTypeInfo::UCHAR:
              pool_type = pool::DbColumn::UCHAR;
              putRange(os, dynamic_cast<NTuple::_Data<unsigned char>*>(it));
              goto MakeCol;
            case DataTypeInfo::USHORT:
              pool_type = pool::DbColumn::USHORT;
              putRange(os, dynamic_cast<NTuple::_Data<unsigned short>*>(it));
              goto MakeCol;
            case DataTypeInfo::UINT:
              pool_type = pool::DbColumn::UINT;
              putRange(os, dynamic_cast<NTuple::_Data<unsigned int>*>(it));
              goto MakeCol;
            case DataTypeInfo::ULONG:
              pool_type = pool::DbColumn::ULONG;
              putRange(os, dynamic_cast<NTuple::_Data<unsigned long>*>(it));
              goto MakeCol;
            case DataTypeInfo::CHAR:
              pool_type = pool::DbColumn::CHAR;
              putRange(os, dynamic_cast<NTuple::_Data<char>*>(it));
              goto MakeCol;
            case DataTypeInfo::SHORT:
              pool_type = pool::DbColumn::SHORT;
              putRange(os, dynamic_cast<NTuple::_Data<short>*>(it));
              goto MakeCol;
            case DataTypeInfo::INT:
              pool_type = pool::DbColumn::INT;
              putRange(os, dynamic_cast<NTuple::_Data<int>*>(it));
              goto MakeCol;
            case DataTypeInfo::LONG:
              pool_type = pool::DbColumn::LONG;
              putRange(os, dynamic_cast<NTuple::_Data<long>*>(it));
              goto MakeCol;
            case DataTypeInfo::BOOL:
              pool_type = pool::DbColumn::BOOL;
              putRange(os, dynamic_cast<NTuple::_Data<bool>*>(it));
              goto MakeCol;
            case DataTypeInfo::FLOAT:
              pool_type = pool::DbColumn::FLOAT;
              putRange(os, dynamic_cast<NTuple::_Data<float>*>(it));
              goto MakeCol;
            case DataTypeInfo::DOUBLE:
              pool_type = pool::DbColumn::DOUBLE;
              putRange(os, dynamic_cast<NTuple::_Data<double>*>(it));
              goto MakeCol;
  MakeCol:
              if ( it->length() == 1 )  {
                pool::DbColumn* col = new pool::DbColumn(it->name(),
                                                         pool_type,
                                                         0,
                                                         DataTypeInfo::size(it->type()));
                item_map[item_no] = cols.size();
                cols.push_back(col);
              }
              // Everything else CANNOT be identified using SQL
            break;
            case DataTypeInfo::UNKNOWN:
            default:
              std::cout << "Create item[FAIL]]: " << it->name()
                        << " Typ:" << it->type() << std::endl;
              break;
          }
          os << '}';
        }
        item_map[item_no] = cols.size();
        pool::DbColumn* col =
          new pool::DbColumn("BlobData",
                             pool::DbColumn::BLOB,
                             0,
                             DataTypeInfo::size(pool::DbColumn::BLOB));
        cols.push_back(col);
        pool::Guid guid;
        std::string cntName = containerName(pRegistry);
        std::string path    = fileName(pRegistry);
        genMD5(os.str(), &guid);
        const pool::DbTypeInfo* typH = pool::DbTypeInfo::createEx(guid, cols);
        StatusCode sc = saveDescription(path,
                                        cntName,
                                        os.str(),
                                        typH->toString(),
                                        guid,
                                        pObj->clID(),
                                        "UPDATE");
        if ( sc.isSuccess() )  {
          // Check if the database container can be connected
          pool::DbContainer cntH(POOL_StorageType);
          StatusCode sc =
            m_dbMgr->connectContainer(IPoolDbMgr::UNKNOWN,
                                      path,
                                      cntName,
                                      pool::RECREATE|pool::UPDATE,
                                      typH,
                                      cntH);
          if ( sc.isSuccess() )  {
            PoolDbTupleCallback* hdlr = new PoolDbTupleCallback(pObj);
            pool::Token*         tok = new pool::Token(cntH.token());
            PoolDbAddress*       add = new PoolDbAddress(tok);
            hdlr->configure(nt, typH, cntH);
            tok->oid().second = 0;
            tok->release();
            pAddr = add;
            add->setHandler(hdlr);
            return m_dbMgr->commitOutput(path, true);
          }
          return S_FAIL;
        }
        return sc;
      }
    }
    else  {
      return S_OK;
    }
  }
  return S_FAIL;
}

/// Resolve the references of the converted object.
StatusCode
PoolDbNTupleCnv::fillRepRefs(IOpaqueAddress* pAddr, DataObject* pObj)
{
  INTuple* pTuple = dynamic_cast<INTuple*>(pObj);
  if ( pTuple )  {
    IRegistry* pRegistry = pObj->registry();
    if ( 0 != pRegistry )  {
      pAddr = pRegistry->address();
      PoolDbAddress* pdbA = dynamic_cast<PoolDbAddress*>(pAddr);
      if ( 0 != pdbA )  {
        PoolDbTupleCallback* cb = dynamic_cast<PoolDbTupleCallback*>(pdbA->handler());
        if ( cb )  {
          const pool::DbTypeInfo* typH = dynamic_cast<const pool::DbTypeInfo*>(cb->shape());
          if ( typH )  {
            if ( bindWrite(pTuple, cb).isSuccess() )  {
              std::string path = fileName(pRegistry);
              std::string cntName = containerName(pRegistry);
              if ( !m_dbMgr->connectOutput(path, "UPDATE").isSuccess() )  {
                MsgStream log(msgSvc(), "PoolDbNTupleCnv");
                log << MSG::ERROR << "Error: connectOutput(path, UPDATE)" << endmsg;
                return S_FAIL;
              }
              if ( m_dbMgr->markWrite(cb->clone(), cntName, &pdbA).isSuccess() ) {
		            if ( m_dbMgr->commitOutput(path, true).isSuccess() )  {
		              typedef INTuple::ItemContainer Cont;
		              Cont& it = pTuple->items();
		              for(Cont::iterator i = it.begin(); i != it.end(); ++i)  {
		                (*i)->reset();
		              }
		              return S_OK;
		            }
              }
            }
          }
        }
      }
    }
  }
  return S_FAIL;
}

StatusCode PoolDbNTupleCnv::bindRead(INTuple* nt,
                                     PoolDbTupleCallback* cb)
{
  typedef INTuple::ItemContainer Cont;
  int cnt = 0;
  //pool::DbBlob&                  s       = cb->stream();
  std::vector<void*>&            addr    = cb->addresses();
  std::vector<PoolDbTokenWrap*>& links   = cb->links();
  const std::vector<int>&        mapping = cb->mapping();
  Cont&                          items   = nt->items();
  for (Cont::iterator i = items.begin(); i != items.end(); ++i, ++cnt )   {
    int count = mapping[cnt];
    if ( count >= 0 )  {
      char* buf = (char*)(*i)->buffer();
      switch( (*i)->type() )   {
        case DataTypeInfo::OBJECT_ADDR:
          addr[count] = &links[cnt];
          break;
        default:
          addr[count] = buf;
          break;
      }
    }
  }
  return S_OK;
}

StatusCode PoolDbNTupleCnv::readData(INTuple* nt,
                                     PoolDbTupleCallback* cb)
{
  typedef INTuple::ItemContainer Cont;
  int cnt = 0;
  pool::DbBlob&                  s       = cb->stream();
  std::vector<PoolDbTokenWrap*>& links   = cb->links();
  const std::vector<int>&        mapping = cb->mapping();
  INTuple::ItemContainer&        items   = nt->items();
  for (Cont::iterator i = items.begin(); i != items.end(); ++i, ++cnt )   {
    int count = mapping[cnt];
    int typ   = (*i)->type();
    if(count < 0 || typ == DataTypeInfo::OBJECT_ADDR || typ == DataTypeInfo::POINTER)  {
      char* buf = (char*)(*i)->buffer();
      int sc = 11;
      switch( (*i)->type() )   {
      case DataTypeInfo::UCHAR:       sc=load<unsigned char> (s,buf); break;
      case DataTypeInfo::USHORT:      sc=load<unsigned short>(s,buf); break;
      case DataTypeInfo::UINT:        sc=load<unsigned int>  (s,buf); break;
      case DataTypeInfo::ULONG:       sc=load<unsigned long> (s,buf); break;
      case DataTypeInfo::CHAR:        sc=load<char>          (s,buf); break;
      case DataTypeInfo::SHORT:       sc=load<short>         (s,buf); break;
      case DataTypeInfo::INT:         sc=load<int>           (s,buf); break;
      case DataTypeInfo::LONG:        sc=load<long>          (s,buf); break;
      case DataTypeInfo::BOOL:        sc=load<bool>          (s,buf); break;
      case DataTypeInfo::FLOAT:       sc=load<float>         (s,buf); break;
      case DataTypeInfo::DOUBLE:      sc=load<double>        (s,buf); break;
      case DataTypeInfo::STRING:      sc=load<std::string>   (s,buf); break;
      case DataTypeInfo::NTCHAR:      sc=load<char*>         (s,buf); break;
      case DataTypeInfo::OBJECT_ADDR: sc=load(buf,links[count]);      break;
      case DataTypeInfo::POINTER:                                     break;
      case DataTypeInfo::UNKNOWN:                                     break;
      default:                                                        break;
      }
      if ( 0 != sc )  {
        MsgStream log(msgSvc(), "PoolDbNTupleCnv");
        log << MSG::DEBUG;
        switch (sc)  {
          case 10:
            log << "CANNOT Set Ntuple token: dynamic_cast<GenericAddress*> is NULL";
          break;
          case 11:
    	      log << "CANNOT Set Ntuple token: invalid address buffer";
          break;
        }
        log << endmsg;
      }
    }
  }
  return S_OK;
}

/// Callback to bind N-tuple data for writing
StatusCode PoolDbNTupleCnv::bindWrite(INTuple* nt, PoolDbTupleCallback* cb)
{
  int cnt = 0;
  pool::DbBlob&                  s       = cb->stream();
  std::vector<void*>&            addr    = cb->addresses();
  std::vector<PoolDbTokenWrap*>& links   = cb->links();
  const std::vector<int>&        mapping = cb->mapping();
  const INTuple::ItemContainer&  items   = nt->items();
  INTuple::ItemContainer::const_iterator i;

  s.setMode(pool::DbBlob::WRITING);
  for(i = items.begin(); i != items.end(); ++i, ++cnt )   {
    char* b     = (char*)(*i)->buffer();
    int   len   = (*i)->filled();
    long  ndim  = (*i)->ndim();
    int   count = mapping[cnt];
    void* ptr = 0;
    switch( (*i)->type() )  {
    case DataTypeInfo::UCHAR:       ptr=save<unsigned char >(s,b,len,ndim>0); break;
    case DataTypeInfo::USHORT:      ptr=save<unsigned short>(s,b,len,ndim>0); break;
    case DataTypeInfo::UINT:        ptr=save<unsigned int  >(s,b,len,ndim>0); break;
    case DataTypeInfo::ULONG:       ptr=save<unsigned long >(s,b,len,ndim>0); break;
    case DataTypeInfo::CHAR:        ptr=save<char          >(s,b,len,ndim>0); break;
    case DataTypeInfo::SHORT:       ptr=save<short         >(s,b,len,ndim>0); break;
    case DataTypeInfo::INT:         ptr=save<int           >(s,b,len,ndim>0); break;
    case DataTypeInfo::LONG:        ptr=save<long          >(s,b,len,ndim>0); break;
    case DataTypeInfo::BOOL:        ptr=save<bool          >(s,b,len,ndim>0); break;
    case DataTypeInfo::FLOAT:       ptr=save<float         >(s,b,len,ndim>0); break;
    case DataTypeInfo::DOUBLE:      ptr=save<double        >(s,b,len,ndim>0); break;
    case DataTypeInfo::STRING:      ptr=save<std::string   >(s,b,len,ndim>0); break;
    case DataTypeInfo::NTCHAR:      ptr=save<char*         >(s,b,len,ndim>0); break;
    case DataTypeInfo::POINTER:     ptr=*(void**)b;                           break;
    case DataTypeInfo::OBJECT_ADDR: ptr=save(b,links[count]);                 break;
    case DataTypeInfo::UNKNOWN:                                               break;
    default:                     /* Do we have to take some action here ? */  break;
    }
    if ( count >= 0 )  {
      addr[count] = ptr;
    }
  }
  return S_OK;
}
