// $Id: TESSerializer.cpp,v 1.2 2008/11/12 23:39:47 marcocle Exp $ 

// 23 May 2009 : changes to dumpBuffer, loadBuffer.
//
// TESSerializer.loadBuffer : Instead of accepting a const buffer, then copying to a new local buffer
// and then rebuilding the TES, the argument is now non-const, and the TES is reconstructed directly
// from the incoming buffer.
// 
// TESSerializer.dumpBuffer : dumpBuffer now accepts an argument; a TBufferFile created externally
// in python.  Sometimes, creating the buffer internally was causing errors when using TMessages and 
// TSockets.  Creating the empty buffer in python and passing as an argument fixes this.

#include "GaudiMP/TESSerializer.h"

// Framework include files
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/GaudiException.h"

// ROOT include files
#include "TROOT.h"
#include "TClass.h"
#include "TInterpreter.h"
#include "TBufferFile.h"

#include <map>
#include <iostream>

namespace {
  struct DataObjectPush {
    DataObjectPush(DataObject*& p) {
      Gaudi::pushCurrentDataObject(&p);
    }
    ~DataObjectPush() {
      Gaudi::popCurrentDataObject();
    }
  };
}

using namespace std;


bool GaudiMP::TESSerializer::analyse(IRegistry* dir, int level)   {
  if ( level < m_currentItem->depth() )   {
    if ( dir->object() != 0 )   {
      m_objects.push_back(dir->object());
      return true;
    }
  }
  return false;
}

GaudiMP::TESSerializer::TESSerializer( IDataProviderSvc* svc )
  : m_TES(svc) 
{
  m_TESMgr = dynamic_cast<IDataManagerSvc*>(svc);
}

void GaudiMP::TESSerializer::dumpBuffer(TBufferFile& buffer) {
  StatusCode status;
  DataObject* obj;

  // static TBufferFile buffer(TBuffer::kWrite);	/* create a buffer */
  static map<string, TClass*> classes;	/* create an STL map to relate string and TClass */

  // Clear current selection 
  m_objects.erase(m_objects.begin(), m_objects.end());

  // Traverse the tree and collect the requested objects
  for ( Items::iterator i = m_itemList.begin(); i != m_itemList.end(); i++ )    {
    m_currentItem = (*i);
    status = m_TES->retrieveObject(m_currentItem->path(), obj);
    if ( status.isSuccess() )  {
      m_TESMgr->traverseSubTree(obj, this);
    }
    else  {
      string text("Cannot serialize mandatory object(s) ");
      throw GaudiException(text + m_currentItem->path(), "", status);
    }
  }
  // Traverse the tree and collect the requested objects (tolerate missing items here)
  for ( Items::iterator i = m_optItemList.begin(); i != m_optItemList.end(); i++ )    {
    m_currentItem = (*i);
    status= m_TES->retrieveObject(m_currentItem->path(), obj);
    if ( status.isSuccess() )  {
      m_TESMgr->traverseSubTree(obj, this);
    }
  }

  // buffer.Reset();
  buffer.WriteInt(m_objects.size());

  for(Objects::iterator i = m_objects.begin(); i != m_objects.end(); ++i) {    
    DataObject* pObj = (*i);	/* define pointer !pObj! to a data object */
    DataObjectPush p(pObj);		/* add the data object to the list... */

    // We build a map so gROOT has to access the whole class database as little as possible
    TClass* cl;						/* announce a TClass */
    const type_info& objClass = typeid(*pObj);		        /* get the type of the data object */
    string objClassName = System::typeinfoName(objClass);	/* and then get the descriptive string from System */

    /* Since we're dealing with a list of objects, we map each one, but if objects are the same, we note
       that they've already been mapped (ie the first 'if case below') */
    if (classes[objClassName])   {
      cl=classes[objClassName];
    } else {
      /* if a class has not been mapped: pull the class name from the objects c_str() method */
      const char* clName = objClassName.c_str();
      /* and find the relevant Tclass (cl) in gROOT */
      cl = gROOT->GetClass(clName);
      /* ... and fill in the map entry */
      classes[objClassName]=cl;
    }

    /* but what if the object is not mapped because gROOT->GetClass cannot find clName? */
    if (cl==0){
      throw GaudiException("gROOT->GetClass cannot find clName", objClassName, StatusCode::FAILURE);      
    }
    // write object to buffer in order location-name-object
    std::string loc=pObj->registry()->identifier();
    buffer.WriteString(loc.c_str());
    buffer.WriteString(cl->GetName());
    cl->Streamer(pObj,buffer);
    /* take care of links */
    LinkManager* linkMgr = pObj->linkMgr();
    int numLinks = linkMgr->size();
    buffer.WriteInt(numLinks);
    // now write each link 
    for (int it = 0; it != numLinks; it++)        {
      const string& link = linkMgr->link(it)->path();
      buffer.WriteString(link.c_str());
    }
  }
  // return &buffer;
}

/* ----------- define loads --------------------- */
void GaudiMP::TESSerializer::loadBuffer(TBufferFile& buffer) {
  
  // reverse mechanism of dumps
  // buffer is: length of DataObjects vector, location string, type name string, the object itself, num of links, list of links

  // not used any more...
  // TBufferFile buffer(TBuffer::kRead, in_buffer.BufferSize()-TMsgReserved, in_buffer.Buffer()+TMsgReserved, false);

  int nObjects;
  buffer.ReadInt(nObjects);

  for (int i=0; i<nObjects; ++i) {
    char text[4096];
    buffer.ReadString(text,sizeof(text));
    string location(text);
    buffer.ReadString(text,sizeof(text));
    TClass* cl = gROOT->GetClass(text);
    
    /// The next is equivalent to ReadObjectAny(cl) except of the 'magic!!'        
    DataObject* obj = (DataObject*)cl->New();
    DataObjectPush push(obj); // This is magic!
    
    cl->Streamer(obj, buffer);
    
    if ( obj ) { // Restore links
      int nlink = 0;
      LinkManager* lnkMgr = obj->linkMgr();
      buffer.ReadInt(nlink);
      
      for (int j=0; j<nlink; ++j) {
        buffer.ReadString(text,sizeof(text));
        lnkMgr->addLink(text,0);
      }
    }
    StatusCode sc;
    sc = m_TES->registerObject(location, obj);
    if (sc.isFailure()) {
      if ( location == "/Event" ) {
        sc = m_TESMgr->setRoot(location, obj);
        if(sc.isFailure()) {
          string msg("Cannot set root at location ");
          throw GaudiException(msg+location,"", sc);
        } 
      }
      else {
        string msg("Cannot register object at location ");
        throw GaudiException(msg+location,"", sc);
      }      
    }
  }
}

// Add item to output to list for serialization (protected)
void GaudiMP::TESSerializer::addItem(Items& itms, const std::string& path, int level)   {
  DataStoreItem* item = new DataStoreItem(path, level);
  itms.push_back( item );
}

// Add item to serialization list; ie append to std::vector of DataStoreItems
void GaudiMP::TESSerializer::addItem(const std::string& path, int level)   {
  addItem( m_itemList, path, level);
}

// Add item to output streamer list
void GaudiMP::TESSerializer::addOptItem(const std::string& path, int level)   {
  addItem( m_optItemList, path, level);
}
