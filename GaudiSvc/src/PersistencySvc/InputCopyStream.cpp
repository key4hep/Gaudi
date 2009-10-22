// $Id: InputCopyStream.cpp,v 1.2 2006/01/10 20:09:27 hmd Exp $
#define GAUDISVC_PERSISTENCYSVC_INPUTCOPYSTREAM_CPP

// Framework include files
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/MsgStream.h"
#include "OutputStreamAgent.h"
#include "InputCopyStream.h"

// Define the algorithm factory for the standard output data writer
DECLARE_ALGORITHM_FACTORY(InputCopyStream)

// Standard Constructor
InputCopyStream::InputCopyStream(const std::string& name, ISvcLocator* pSvcLocator)
 : OutputStream(name, pSvcLocator)
{
  m_doPreLoad      = false;
  m_doPreLoadOpt   = false;
  m_itemNames.push_back("/Event#99999");
  declareProperty("TakeOptionalFromTES", m_takeOptionalFromTES = false, 
                  "Allow optional items to be on TES instead of input file") ;
}

// Standard Destructor
InputCopyStream::~InputCopyStream()   {
}

// Place holder to create configurable data store agent
StatusCode InputCopyStream::collectLeaves(IRegistry* dir, int level)    {
  MsgStream log(msgSvc(), name());
  if ( level < m_currentItem->depth() )   {
    if ( dir )  {
      // dir->object != 0, because was retrived in previous recursion
      m_objects.push_back(dir->object());
      if ( dir->address() )  {
        std::vector<IRegistry*> lfs;
        const std::string& dbase = dir->address()->par()[0];
        // Cololect all pending leaves
        StatusCode iret, sc = m_pDataManager->objectLeaves(dir,lfs);
        if ( sc.isSuccess() )  {
          std::vector<IRegistry*>::iterator i=lfs.begin();
          for(; i!=lfs.end(); ++i)  {
            // Continue if the leaf has the same database as the parent
            if ( (*i)->address() && (*i)->address()->par()[0] == dbase )  {
              DataObject* obj = 0;
              iret = m_pDataProvider->retrieveObject(dir, (*i)->name(), obj);
              if (  iret.isSuccess() )  {
                log << MSG::VERBOSE << "::collectLeaves Success retrieving " << (*i)->name() << endmsg ;
                iret = collectLeaves(*i, level+1);
              }
              if ( !iret.isSuccess() )  {
                log << MSG::VERBOSE << "::collectLeaves Failure retrieving " << (*i)->name() << endmsg ;
                sc = iret;
              }
            }
          }
        }
        return sc;
      }
    }
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

/// Collect all objects to be written tio the output stream
StatusCode InputCopyStream::collectObjects()   {
  MsgStream log(msgSvc(), name());
  StatusCode status = StatusCode::SUCCESS;
  Items::iterator i;
  // Traverse the tree and collect the requested objects
  for ( i = m_itemList.begin(); i != m_itemList.end(); i++ )    {
    DataObject* obj = 0;
    m_currentItem = (*i);
    log << MSG::VERBOSE << "::collectObjects Looping over mandatory " << m_currentItem->path() << endmsg ;
    StatusCode iret = m_pDataProvider->retrieveObject(m_currentItem->path(), obj);
    if ( iret.isSuccess() )  {
      log << MSG::VERBOSE << "::collectObjects Success retrieving mandatory " << (*i)->path() << endmsg ;
      iret = collectLeaves(obj->registry(), 0);
    }
    if ( !iret.isSuccess() )  {
      log << MSG::ERROR << "Cannot write mandatory object(s) (Not found) "
          << m_currentItem->path() << endmsg;
      status = iret;
    }
  }
  // Traverse the tree and collect the requested objects (tolerate missing itmes here)
  for ( i = m_optItemList.begin(); i != m_optItemList.end(); i++ )    {
    DataObject* obj = 0;
    m_currentItem = (*i);
    StatusCode iret = m_pDataProvider->retrieveObject(m_currentItem->path(), obj);
    if ( iret.isSuccess() )  {
      log << MSG::VERBOSE << "::collectObjects Success retrieving optional " << (*i)->path() << endmsg ;
      if ( m_takeOptionalFromTES ){ // look into TES
        iret = m_pDataManager->traverseSubTree(obj, m_agent);      
      } else { // look only at DST
        iret = collectLeaves(obj->registry(), 0);
      }
    }
    if ( !iret.isSuccess() )    {
      log << MSG::DEBUG << "Ignore request to write non-mandatory object(s) "
          << m_currentItem->path() << endmsg;
    }
  }
  return status;
}
