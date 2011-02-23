// $Id $
// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/ObjectContainerBase.h"
#include "GaudiKernel/ContainedObject.h"

/**

  Namespace for basic object handling within container classes

*/
namespace Objects    {
  bool access(const ContainedObject* from, ContainedObject** to);
  bool access(const DataObject* from, DataObject** to);

  template <class TO, class FROM>
  TO* reference(FROM* from)    {
    ContainedObject* to = 0;
    if ( access(from, &to) )    {
      return dynamic_cast<TO*>(to);
    }
    return 0;
  }
}

bool Objects::access(const DataObject* from, DataObject** to)
{
  DataObject* src  = const_cast<DataObject*>(from);
  DataObject* tar = 0;
  if ( src != 0 )
  {
    LinkManager* mgr = src->linkMgr();
    if ( 0 != mgr )
    {
      LinkManager::Link* link = mgr->link(long(0));
      if ( 0 != link )
      {
        tar = link->object();
        if ( 0 == tar )
        {
          IRegistry* reg = src->registry();
          if ( 0 != reg )
          {
            IDataProviderSvc* ds = reg->dataSvc();
            if ( 0 != ds )
            {
              if ( ds->retrieveObject(link->path(), tar).isSuccess() )
              {
                link->setObject(tar);
              }
            }
          }
        }
      }
    }
  }
  *to = tar;
  return tar != 0;
}

// Load on demand: ContainedObject type references
bool Objects::access(const ContainedObject* from, ContainedObject** to)
{
  *to = 0;
  if ( from )
  {
    DataObject *tar = 0;
    if ( access(from->parent(), &tar) )
    {
      ObjectContainerBase* cnt = dynamic_cast<ObjectContainerBase*>(tar);
      if ( cnt )
      {
        *to = cnt->containedObject(from->index());
      }
    }
  }
  return (*to) != 0;
}

