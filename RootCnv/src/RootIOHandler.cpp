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
//====================================================================
//
//  Package    : RootCnv
//
//  Description: Generate TClassStreamers for special Gaudi classes
//  @author      M.Frank
//====================================================================
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/LinkManager.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/ObjectContainerBase.h>
#include <GaudiKernel/SmartRef.h>
#include <GaudiKernel/System.h>
#include <RootCnv/PoolClasses.h>
#include <RootCnv/RootRefs.h>
#include <TClass.h>
#include <TClassStreamer.h>
#include <TInterpreter.h>
#include <TROOT.h>
#include <TSystem.h>
#include <iostream>
#include <memory>
#include <stdexcept>

using namespace std;

namespace GaudiRoot {
  static const DataObject* last_link_object = nullptr;
  static int               last_link_hint   = -1;
  void                     resetLastLink() {
    last_link_object = nullptr;
    last_link_hint   = -1;
  }
  void pushCurrentDataObject( DataObject** pobjAddr ) {
    Gaudi::pushCurrentDataObject( pobjAddr );
    resetLastLink();
  }
  void popCurrentDataObject() {
    Gaudi::popCurrentDataObject();
    resetLastLink();
  }

  union RefAccessor {
    void*                      Ptr;
    SmartRefBase*              Base;
    SmartRef<DataObject>*      ObjectRef;
    SmartRef<ContainedObject>* ContainedRef;
    RefAccessor( void* obj ) { Ptr = obj; }
  };

  /**@class IOHandler
   */
  template <class T>
  class IOHandler : public TClassStreamer {
  protected:
    /// ROOT persistent class description
    TClass* m_root;

  public:
    /// Initializing constructor
    IOHandler( TClass* c ) : m_root( c ) {}
    /// ROOT I/O callback
    void operator()( TBuffer& b, void* obj ) override {
      try {
        if ( b.IsReading() )
          get( b, obj );
        else
          put( b, obj );
      } catch ( const exception& e ) {
        string err = "Class:" + string( m_root->GetName() ) + "> Exception in object I/O";
        err += e.what();
        throw runtime_error( err );
      } catch ( ... ) {
        string err = "Class:" + string( m_root->GetName() ) + "> Exception in object I/O";
        throw runtime_error( err );
      }
    }
    /// Callback for reading the object
    virtual void get( TBuffer& b, void* obj );
    /// Callback for writing the object
    virtual void put( TBuffer& b, void* obj );
  };

  template <>
  void IOHandler<SmartRefBase>::get( TBuffer& b, void* obj ) {
    RefAccessor r( obj );
    UInt_t      start, count;
    Version_t   version = b.ReadVersion( &start, &count, m_root );
    m_root->ReadBuffer( b, obj, version, start, count );
    switch ( r.Base->objectType() ) {
    case SmartRefBase::DATAOBJECT:
      ( *r.ObjectRef )( Gaudi::getCurrentDataObject() );
      break;
    case SmartRefBase::CONTAINEDOBJECT:
      ( *r.ContainedRef )( Gaudi::getCurrentDataObject() );
      break;
    default:
      cout << "Hit uninitialized smartRef!!!!" << endl;
      break;
    }
  }

  template <>
  void IOHandler<SmartRefBase>::put( TBuffer& b, void* obj ) {
    RefAccessor      r( obj );
    ContainedObject* p;
    DataObject *     curr, *pDO;
    int              hint, link;

    curr = Gaudi::getCurrentDataObject();
    pDO  = r.ObjectRef->data();
    hint = r.ObjectRef->hintID();
    link = r.ObjectRef->linkID();
    if ( pDO ) {
      switch ( r.Base->objectType() ) {
      case SmartRefBase::CONTAINEDOBJECT:
        p = r.ContainedRef->data();
        if ( p ) {
          const ObjectContainerBase* parent = p->parent();
          if ( parent ) {
            link = p->index();
            pDO  = const_cast<ObjectContainerBase*>( parent );
            break;
          }
        }
        pDO = nullptr;
        cout << "IOHandler<SmartRefBase>::onWrite> "
             << "Found invalid smart reference with object "
             << "having no parent." << endl;
        throw runtime_error( "IOHandler<SmartRefBase>::onWrite> "
                             "Found invalid smart reference with object "
                             "having no parent." );
        break;
      case SmartRefBase::DATAOBJECT:
        link = StreamBuffer::INVALID;
        break;
      default:
        break;
      }
      if ( pDO == last_link_object ) {
        r.Base->set( curr, last_link_hint, link );
        m_root->WriteBuffer( b, obj );
        return;
      } else if ( pDO ) {
        LinkManager* mgr = curr->linkMgr();
        IRegistry*   reg = pDO->registry();
        if ( reg && mgr ) {
          hint             = mgr->addLink( reg->identifier(), pDO );
          last_link_hint   = hint;
          last_link_object = pDO;
        }
      } else {
        hint = link = StreamBuffer::INVALID;
      }
    }
    r.Base->set( curr, hint, link );
    m_root->WriteBuffer( b, obj );
  }

  template <>
  void IOHandler<ContainedObject>::get( TBuffer& b, void* obj ) {
    UInt_t    start, count;
    Version_t version = b.ReadVersion( &start, &count, m_root );
    m_root->ReadBuffer( b, obj, version, start, count );
    ContainedObject* p = (ContainedObject*)obj;
    p->setParent( (ObjectContainerBase*)Gaudi::getCurrentDataObject() );
  }

  template <>
  void IOHandler<ContainedObject>::put( TBuffer& b, void* obj ) {
    m_root->WriteBuffer( b, obj );
  }

  template <>
  void IOHandler<pool::Token>::get( TBuffer& b, void* obj ) {
    UInt_t       start, count;
    pool::Token* t = (pool::Token*)obj;
    b.ReadVersion( &start, &count, m_root );
    b.ReadFastArray( &t->m_oid.first, 2 );
    b.CheckByteCount( start, count, m_root );
  }

  template <>
  void IOHandler<pool::Token>::put( TBuffer&, void* ) {
    throw runtime_error( "Writing POOL files is not implemented!" );
  }

  template <class T>
  static bool makeStreamer( MsgStream& log ) {
    string  cl_name = System::typeinfoName( typeid( T ) );
    TClass* c       = gROOT->GetClass( cl_name.c_str() );
    if ( c ) {
      c->AdoptStreamer( new IOHandler<T>( c ) );
      log << MSG::DEBUG << "Installed IOHandler for class " << cl_name << endmsg;
      return true;
    }
    log << MSG::ERROR << "[No ROOT TClass] Failed to install IOHandler for class " << cl_name << endmsg;
    return false;
  }

  bool patchStreamers( MsgStream& s ) {
    static bool first = true;
    if ( first ) {
      first = false;
#if ROOT_VERSION_CODE < ROOT_VERSION( 5, 99, 0 )
      gSystem->Load( "libCintex" );
      gROOT->ProcessLine( "Cintex::Cintex::Enable()" );
      gROOT->ProcessLine( "#include <vector>" );
      gInterpreter->EnableAutoLoading();
      gInterpreter->AutoLoad( "DataObject" );
      gInterpreter->AutoLoad( "PoolDbLinkManager" );
      gSystem->Load( "libGaudiKernelDict" );
      gSystem->Load( "libGaudiTestSuiteDict" );
#else
#  if ROOT_VERSION_CODE < ROOT_VERSION( 6, 19, 0 )
      gInterpreter->EnableAutoLoading();
#  else
      gInterpreter->LoadLibraryMap();
#  endif
      gInterpreter->AutoLoad( "DataObject" );
      gInterpreter->AutoLoad( "PoolDbLinkManager" );
#endif

      bool b1 = makeStreamer<SmartRefBase>( s );
      bool b2 = makeStreamer<ContainedObject>( s );
      bool b3 = makeStreamer<pool::Token>( s );
      return b1 && b2 && b3;
    }
    return true;
  }
} // namespace GaudiRoot
