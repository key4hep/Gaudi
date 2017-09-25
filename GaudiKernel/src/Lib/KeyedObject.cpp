// Include files
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/ObjectContainerBase.h"

/**

  Namespace for basic object handling within container classes

*/
namespace Objects
{
  bool access( const ContainedObject* from, ContainedObject** to );
  bool access( const DataObject* from, DataObject** to );

  template <class TO, class FROM>
  TO* reference( FROM* from )
  {
    ContainedObject* to = nullptr;
    return access( from, &to ) ? dynamic_cast<TO*>( to ) : nullptr;
  }
}

bool Objects::access( const DataObject* from, DataObject** to )
{
  DataObject* src = const_cast<DataObject*>( from );
  DataObject* tar = nullptr;
  if ( src ) {
    LinkManager* mgr = src->linkMgr();
    if ( mgr ) {
      LinkManager::Link* link = mgr->link( long( 0 ) );
      if ( link ) {
        tar = link->object();
        if ( !tar ) {
          IRegistry* reg = src->registry();
          if ( reg ) {
            IDataProviderSvc* ds = reg->dataSvc();
            if ( ds && ds->retrieveObject( link->path(), tar ).isSuccess() ) {
              link->setObject( tar );
            }
          }
        }
      }
    }
  }
  *to = tar;
  return tar != nullptr;
}

// Load on demand: ContainedObject type references
bool Objects::access( const ContainedObject* from, ContainedObject** to )
{
  *to = nullptr;
  if ( from ) {
    DataObject* tar = nullptr;
    if ( access( from->parent(), &tar ) ) {
      ObjectContainerBase* cnt = dynamic_cast<ObjectContainerBase*>( tar );
      if ( cnt ) {
        *to = cnt->containedObject( from->index() );
      }
    }
  }
  return *to != nullptr;
}
