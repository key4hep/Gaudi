#include "MyTrack.h"
#include "GaudiKernel/KeyedContainer.h"
#include "GaudiPoolDb/PoolDbKeyedContainerCnv.h"
#include "GaudiKernel/CnvFactory.h"

/*
 * Namespace declaration
 */
namespace Tests  {
  /** @class MyTrackContainerCnv MyTrackContainerCnv.h src/MyTrackContainerCnv .h
    *
    *  Description:
    *  Example for a specialized converter....
    *
    *  Author:  M.Frank
    */
  class MyTrackContainerCnv : public PoolDbKeyedContainerCnv    {
  public:
    typedef KeyedContainer<MyTrack> container_t;

    /// Standard Constructor
    MyTrackContainerCnv(const long typ,const CLID& clid, ISvcLocator* svc)
    : PoolDbKeyedContainerCnv(typ,clid,svc) {                                 }
    /// Standard Destructor
    virtual ~MyTrackContainerCnv()          {                                 }
    /// Container's class ID (needed by the converter factory!)
    static const CLID& classID()            {  return container_t::classID(); }
    /// Update the references of an updated transient object.
    virtual StatusCode updateObjRefs(IOpaqueAddress* pAddr, DataObject* pObj)  {
      StatusCode sc = PoolDbKeyedContainerCnv::updateObjRefs(pAddr, pObj);
      if ( sc.isSuccess() )  {
        container_t* c = dynamic_cast<container_t*>(pObj);
        if ( c )  {
          MsgStream log(msgSvc(),"MyTrackContainerCnv");
          log << MSG::ALWAYS << "Called specialized track converter." << endmsg;
          // Do container manipulations right here

          return sc;
        }
        sc = StatusCode::FAILURE;
      }
      return sc;
    }
  };

// Keyed HashMap factory

PLUGINSVC_FACTORY_WITH_ID( MyTrackContainerCnv,
                           ConverterID(POOL_StorageType,KeyedContainer<MyTrack>::classID()),
                           IConverter*(long, CLID, ISvcLocator*) );
}
