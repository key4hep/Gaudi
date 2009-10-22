// $Id: IPartitionControl.h,v 1.1 2004/07/06 10:17:40 mato Exp $
#ifndef GAUDIKERNEL_IPARTITIONCONTROL_H
#define GAUDIKERNEL_IPARTITIONCONTROL_H

// Framework include files
#include "GaudiKernel/IInterface.h"

// C++ include files
#include <string>

/**@class IPartitionControl IPartitionControl.h GaudiKernel/IPartitionControl.h
  *
  * Create / access partitions.
  * Partitioning is used to switch between different instances of
  * multi-services like it is used e.g. in the event buffer tampering.
  *
  *
  * Example code:
  * =============
  *
  * Switch between buffers for data processing.
  * Use this technique to populate e.g. different datastores
  * in order to later compare the different results.
  *
  * SmartIF<IPartitionControl> partCtrl(eventSvc());
  * if ( partCtrl.isValid() )  {
  *   if ( partCtrl.activate("Partition_1").isSuccess() )  {
  *      SmartDataPtr mcparts(eventSvc(), "MC/Particles");
  *      .... work with particles from buffer "Partition 1"
  *      all data registered by code here will go to "Partition 2"
  *   }
  *   if ( partCtrl.activate("Partition_2").isSuccess() )  {
  *      SmartDataPtr mcparts(eventSvc(), "MC/Particles");
  *      .... work with particles from buffer "Partition 2"
  *      they are NOT they same as those in buffer 1
  *      all data registered by code here will go to "Partition 2"
  *   }
  * }
  *
  * The access to the underlying service is also possible.
  *
  * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  *
  * Be careful: This usage MAY NOT MODIFY e.g. the datastore
  * by adding additional objects! This should solely by
  * used for analysis after buffer tampering. In particular
  * with "data on demand" this can easily have unexpected results.
  *
  * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  *
  * SmartIF<IPartitionControl> partCtrl(eventSvc());
  * if ( partCtrl )  {
  *   IInterface* ptr = 0;
  *   if ( partCtrl->get("Partition 1", ptr).isSuccess() )  {
  *     SmartIF<IDataProviderSvc> partition(ptr);
  *     // Now we can work directly with this partition
  *     // There is no additional dispathing in the "eventSvc()".
  *     SmartDataPtr mcparts(partition, "MC/Particles");
  *     if ( mcparts )  {
  *     }
  *   }
  * }
  *
  *  @author  Markus Frank
  *  @version 1.0
  */
class GAUDI_API IPartitionControl: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IPartitionControl,1,0);

  /// Create a partition object. The name identifies the partition uniquely
  /** Create a new partition. If the partition exists already
    * the existing object is returned. In this event the return code is
    * PARTITION_EXISTS. The partition type typically contains all the
    * information necessary to access the corresponding service,
    * typically a pair "<service-type>/<service name>"
    *
    * @param  name          [IN]     Partition name
    * @param  type          [IN]     Partition type
    *
    * @return Status code indicating failure or success.
    */
  virtual StatusCode create(const std::string& name,
                            const std::string& type) = 0;

  /// Create a partition object. The name identifies the partition uniquely
  /** Create a new partition. If the partition exists already
    * the existing object is returned. In this event the return code is
    * PARTITION_EXISTS. The partition type typically contains all the
    * information necessary to access the corresponding service,
    * typically a pair "<service-type>/<service name>"
    *
    * @param  name          [IN]     Partition name
    * @param  type          [IN]     Partition type
    * @param  pPartition    [OUT]    Reference to the created/existing partition
    *
    * @return Status code indicating failure or success.
    */
  virtual StatusCode create(const std::string& name,
                            const std::string& type,
                            IInterface*& pPartition) = 0;

  /// Drop a partition object. The name identifies the partition uniquely
  /** Remove a partition object. If the partition object does not exists,
    * the return code is PARTITION_NOT_PRESENT. The partition may not
    * be used anymore after this call.
    *
    * @param  name          [IN]     Partition name
    *
    * @return Status code indicating failure or success.
    */
  virtual StatusCode drop(const std::string& name) = 0;

  /// Drop a partition object. The name identifies the partition uniquely
  /** Remove a partition object. If the partition object does not exists,
    * the return code is PARTITION_NOT_PRESENT. The partition may not
    * be used anymore after this call.
    * If the interface pointer is invalid, IInterface::NO_INTERFACE
    * is returned.
    *
    * @param  pPartition    [IN]     Reference to existing partition
    *
    * @return Status code indicating failure or success.
    */
  virtual StatusCode drop(IInterface* pPartition) = 0;

  /// Activate a partition object. The name identifies the partition uniquely.
  /** Access an existing partition object. Preferred call.
    * The activation of a partition does not change the state of the
    * partition. It only means that any call issued to the corresponding
    * multi-service will be redirected to the this partition - typically
    * a service implementing the same interfaces as the multi-service.
    *
    * @param  name          [IN]     Partition name
    *
    * @return Status code indicating failure or success.
    */
  virtual StatusCode activate(const std::string& name) = 0;

  /// Activate a partition object.
  /** Access an existing partition object.
    * The activation of a partition does not change the state of the
    * partition. It only means that any call issued to the corresponding
    * multi-service will be redirected to the this partition - typically
    * a service implementing the same interfaces as the multi-service.
    *
    * If the interface pointer is invalid, IInterface::NO_INTERFACE
    * is returned.
    *
    * @param  pPartition    [IN]     Pointer to the partition.
    *
    * @return Status code indicating failure or success.
    */
  virtual StatusCode activate(IInterface* pPartition) = 0;

  /// Access a partition object. The name identifies the partition uniquely.
  /** Access an existing partition object.
    *
    * @param  name          [IN]     Partition name
    * @param  pPartition    [IN]     Location to store the pointer to the partition.
    *
    * @return Status code indicating failure or success.
    */
  virtual StatusCode get(const std::string& name, IInterface*& pPartition) const = 0;

  /// Access the active partition object.
  /** Access the active partition object.
    *
    * @param  name          [OUT]    Partition name
    * @param  pPartition    [OUT]    Location to store the pointer to the partition.
    *
    * @return Status code indicating failure or success.
    */
  virtual StatusCode activePartition(std::string& name, IInterface*& pPartition) const = 0;

  // Return codes:
  enum {  PARTITION_NOT_PRESENT = 2,
          PARTITION_EXISTS = 4,
          NO_ACTIVE_PARTITION = 6
  };
};
#endif // GAUDIKERNEL_IPARTITIONCONTROL_H
