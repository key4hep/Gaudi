// $Id: PoolDbIOHandler.cpp,v 1.13 2008/11/12 23:39:47 marcocle Exp $
//====================================================================
//
//	Package    : System (The POOL project)
//
//  Description: Generate neccessary infrastructure for the
//               SEAL plugin manager modules
//
//	@author      M.Frank
//====================================================================
#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, coming from ROOT
#pragma warning(disable:2259)
#endif

#include "GaudiPoolDb/PoolDbIOHandler.h"
#include "GaudiKernel/ObjectContainerBase.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartRef.h"
#include <stdexcept>
#include <iostream>
#include "TROOT.h"
#include "TFile.h"
#include "TClass.h"
#include "TStreamerInfo.h"
#include "Cintex/Cintex.h"
#include "POOLCore/Token.h"
#include "POOLCore/Reference.h"
#include "StorageSvc/DbReflex.h"
#include "StorageSvc/DataCallBack.h"
#include "StorageSvc/DbDataHandlerGuard.h"

#include <memory>

using namespace pool;
using namespace ROOT::Reflex;
using namespace ROOT::Cintex;

namespace ROOT { namespace Cintex  {
  bool IsTypeOf(Type& typ, const std::string& base_name);
}}
namespace pool  {
  const std::string typeName(const std::type_info& typ);
}


static const DataObject* last_link_object = 0;
static int               last_link_hint = -1;

void resetLastLink() {
  last_link_object = 0;
  last_link_hint   = -1;
}

using Gaudi::getCurrentDataObject;

void pushCurrentDataObject(DataObject** pobjAddr) {
  Gaudi::pushCurrentDataObject(pobjAddr);
  resetLastLink();
}

void popCurrentDataObject() {
  Gaudi::popCurrentDataObject();
  resetLastLink();
}


template <class T>
void PoolDbIOHandler<T>::operator()(TBuffer &b, void *obj)  {
  try {
    if ( b.IsReading() )   {
      get(b,obj);
    }
    else  {
      put(b,obj);
    }
  }
  catch( const std::exception& e )    {
    std::string err = "Class:" + std::string(m_root->GetName()) + "> Exception in object I/O";
    err += e.what();
    throw std::runtime_error(err);
  }
  catch( ... )    {
    std::string err = "Class:" + std::string(m_root->GetName()) + "> Exception in object I/O";
    throw std::runtime_error(err);
  }
}

template <>
void PoolDbIOHandler<SmartRefBase>::get(TBuffer &b, void* obj) {
  UInt_t start, count;
  SmartRefBase* ref = (SmartRefBase*)obj;
  Version_t version = b.ReadVersion(&start, &count, m_root);
  m_root->ReadBuffer(b, obj, version, start, count);
  switch( ref->objectType() ) {
   case SmartRefBase::DATAOBJECT:
    {
      SmartRef<DataObject>* r = (SmartRef<DataObject>*)obj;
      (*r)(getCurrentDataObject());
    }
    break;
  case SmartRefBase::CONTAINEDOBJECT:
   {
     SmartRef<ContainedObject>* r = (SmartRef<ContainedObject>*)obj;
     (*r)(getCurrentDataObject());
   }
   break;
  default:
    std::cout << "Hit uninitialized smartRef!!!!" << std::endl;
    break;
  }
}

template <>
void PoolDbIOHandler<SmartRefBase>::put(TBuffer &b, void* obj) {
  SmartRefBase* ref = (SmartRefBase*)obj;
  SmartRef<DataObject>* r1 = (SmartRef<DataObject>*)ref;
  DataObject* curr = getCurrentDataObject();
  DataObject* pDO  = r1->data();
  int hint = r1->hintID();
  int link = r1->linkID();
  if ( pDO )  {
    switch( ref->objectType() ) {
    case SmartRefBase::CONTAINEDOBJECT:
      {
        SmartRef<ContainedObject>* r2 = (SmartRef<ContainedObject>*)ref;
        ContainedObject* p = r2->data();
        if ( p )  {
          const ObjectContainerBase* parent = p->parent();
          if ( parent )  {
            link = p->index();
            pDO  = const_cast<ObjectContainerBase*>(parent);
            break;
          }
        }
      }
      pDO = 0;
      std::cout << "PoolDbIOHandler<SmartRefBase>::onWrite> "
                << "Found invalid smart reference with object "
                << "having no parent."
                << std::endl;
      throw std::runtime_error("PoolDbIOHandler<SmartRefBase>::onWrite> "
                               "Found invalid smart reference with object "
                               "having no parent.");
      break;
    case SmartRefBase::DATAOBJECT:
      link = StreamBuffer::INVALID;
      break;
    default:
      break;
    }
    //if ( pDO == last_link_object && last_link_hint == -1 )  {
    //  std::cout << "PoolDbIOHandler<SmartRefBase>::onWrite> "
    //            << "Found invalid smart reference."
    //            << std::endl;
    //}
    if ( pDO == last_link_object )  {
      ref->set(curr, last_link_hint, link);
      m_root->WriteBuffer(b, obj);
      return;
    }
    else {
      LinkManager* mgr = curr->linkMgr();
      IRegistry*   reg = pDO->registry();
      if ( reg && mgr )  {
        hint = mgr->addLink(reg->identifier(), pDO);
        last_link_hint   = hint;
        last_link_object = pDO;
      }
    }
  }
  //if ( hint == StreamBuffer::INVALID )  {
  //  std::cout << "PoolDbIOHandler<SmartRefBase>::onWrite> "
  //            << "Found invalid smart reference."
  //            << std::endl;
  //}
  ref->set(curr, hint, link);
  m_root->WriteBuffer(b, obj);
}

template <>
void PoolDbIOHandler<ContainedObject>::get(TBuffer &b, void* obj) {
  UInt_t start, count;
  Version_t version = b.ReadVersion(&start, &count, m_root);
  m_root->ReadBuffer(b, obj, version, start, count);
  ContainedObject* p = (ContainedObject*)obj;
  p->setParent((ObjectContainerBase*)getCurrentDataObject());
}

template <>
void PoolDbIOHandler<ContainedObject>::put(TBuffer &b, void* obj) {
  m_root->WriteBuffer(b, obj);
}

// required for backward compatibility
#ifndef ROOT_FULL_VERSION
#define ROOT_FULL_VERSION(a,b,c,p) (((a)<<24)+((b)<<16)+((c)<<8)+(p))
#define ROOT_FULL_VERSION_CODE (ROOT_VERSION_CODE << 8)
#endif

static void getOID_40000(TBuffer& b, TClass* cl, Token::OID_t& oid) {
  unsigned long loid[2];
  UInt_t start, count;
#if (ROOT_FULL_VERSION_CODE < ROOT_FULL_VERSION(5,28,0,'a'))
  UInt_t tmp;
#endif
  // read the class version from the buffer
  /* Version_t vsn = */ b.ReadVersion(&start, &count, 0);
  switch(count) {
  case 22:              // These tokens were written as pair<long,long>
#if (ROOT_FULL_VERSION_CODE < ROOT_FULL_VERSION(5,28,0,'a'))
    b >> tmp;           //
#endif
    b.ReadFastArray(loid,2); // There was a bug in POOL....
    oid.first = loid[0];
    oid.second = loid[1];
    break;              // see TBuffer::ReadFastArray(ulong*, int)
#if (ROOT_FULL_VERSION_CODE < ROOT_FULL_VERSION(5,28,0,'a'))
  case 14:              // Normal case: version:checksum+8 Bytes
    b >> tmp;           //
  case 10:              // Normal case: version:checksum+8 Bytes
  case 8:               // Without checksum and version
#endif
  default:              // No better idea
    b.ReadFastArray(&oid.first, 2);
    break;
  }
  // Check that the buffer position correesponds to the byte count
  b.CheckByteCount(start, count, cl);
}

template <>
void PoolDbIOHandler<Token>::get(TBuffer& b, void* obj) {
  Token* t = (Token*)obj;
  DataCallBack* caller = DbDataHandlerGuard::caller();
  Int_t file_version = ((TFile*)b.GetParent())->GetVersion();
  if ( file_version >= 40000 ) {
    getOID_40000(b, m_root, t->oid());
  }
  else {
    UInt_t start, count, tmp;
    Version_t vsn = b.ReadVersion(&start, &count, m_root);
    switch(vsn)  {
    case 2:
      b >> tmp;
      b.ReadFastArray(&t->oid().first, 2);
      b >> tmp;
      break;
    default:
      b.SetBufferOffset(start+4);
      b.ReadFastArray(&t->oid().first, 2);
      break;
    }
  }
  if (caller) caller->notify(DataCallBack::GET,DataCallBack::TOKEN,m_type,obj,&t->oid());
}

template <>
void PoolDbIOHandler<Token>::put(TBuffer &b, void* obj) {
  Token::OID_t* poid = &(((Token*)obj)->oid());
  UInt_t count = b.WriteVersion(m_root, true);
  DataCallBack* caller = DbDataHandlerGuard::caller();
  if (caller) caller->notify(DataCallBack::PUT,DataCallBack::TOKEN,m_type,obj,poid);
  b.WriteFastArray(&poid->first, 2);
  b.SetByteCount(count, true);
}

template <>
void PoolDbIOHandler<Reference>::get(TBuffer& b, void* obj) {
  Token::OID_t oid(~0x0,~0x0);
  DataCallBack* caller = DbDataHandlerGuard::caller();
  Int_t file_version = ((TFile*)b.GetParent())->GetVersion();
  (file_version >= 40000) ? getOID_40000(b, m_root,oid) : b.ReadFastArray(&oid.first, 2);
  if (caller) caller->notify(DataCallBack::GET,DataCallBack::REFERENCE,m_type,obj,&oid);
}

template <>
void PoolDbIOHandler<Reference>::put(TBuffer &b, void* obj) {
  UInt_t count = b.WriteVersion(m_root, true);
  DataCallBack* caller = DbDataHandlerGuard::caller();
  Token::OID_t oid(~0x0, ~0x0);
  if (caller) caller->notify(DataCallBack::PUT,DataCallBack::REFERENCE,m_type,obj,&oid);
  b.WriteFastArray(&oid.first, 2);
  b.SetByteCount(count, true);
}

template <class T> static bool makeStreamer(MsgStream& log)  {
  std::string cl_name = typeName(typeid(T));
  ROOT::Reflex::Type t = ROOT::Reflex::Type::ByName(cl_name);
  if ( t )  {
    TClass* c = gROOT->GetClass(cl_name.c_str());
    if ( c ) {
      TClassStreamer* s = new PoolDbIOHandler<T>(t,c);
      c->AdoptStreamer(s);
      log << MSG::DEBUG << "Installed IOHandler for class " << cl_name << endmsg;
      return true;
    }
    else if ( !c )  {
      log << MSG::ERROR << "[No ROOT TClass] Failed to install IOHandler for class " << cl_name << endmsg;
    }
  }
  else  {
    log << MSG::ERROR << "[No Reflection Type] Failed to install IOHandler for class " << cl_name << endmsg;
  }
  return false;
}

namespace GaudiPoolDb  {
  bool patchStreamers(MsgStream& s)  {
    static bool first = true;
    if ( first ) {
      first = false;
      for ( Type_Iterator i=Type::Type_Begin(); i != Type::Type_End(); ++i)   {
        Type typ = *i;
        if ( !(typ.IsStruct() || typ.IsClass()) )  continue;
        TClass* cl = 0;
	if ( IsTypeOf(typ,"pool::Reference") )  {
	  cl = gROOT->GetClass(typ.Name(SCOPED).c_str());
	  if ( cl ) cl->AdoptStreamer(new PoolDbIOHandler<Reference>(typ,cl));
	}
	else if ( IsTypeOf(typ,"pool::Token") )  {
	  cl = gROOT->GetClass(typ.Name(SCOPED).c_str());
	  if ( cl ) cl->AdoptStreamer(new PoolDbIOHandler<Token>(typ,cl));
	}
      }
      ROOT::Cintex::Cintex::Enable();
      bool b2 = makeStreamer<SmartRefBase>(s);
      bool b3 = makeStreamer<ContainedObject>(s);
      return b2 && b3;
    }
    return true;
   }
}
