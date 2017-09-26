#ifndef GAUDIKERNEL_IHIVEWHITEBOARD_H
#define GAUDIKERNEL_IHIVEWHITEBOARD_H

// Framework include files
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/IInterface.h"

// C++ include files
#include <string>

/**@class IHiveWhiteBoard IHiveWhiteBoard.h GaudiKernel/IHiveWhiteBoard.h
 *
 *  @author  Pere Mato
 *  @author  Danilo Piparo
 *  @version 1.0
 */
class GAUDI_API IHiveWhiteBoard : virtual public IInterface
{
public:
  /// InterfaceID
  DeclareInterfaceID( IHiveWhiteBoard, 1, 0 );

  /** Activate an given 'slot' for all subsequent calls within the
   * same thread id.
   *
   * @param  partition     [IN]     Partition number (event slot)   *
   * @return Status code indicating failure or success.
   */
  virtual StatusCode selectStore( size_t partitionIndex ) = 0;

  /** Clear an given 'slot'.
   *
   * @param  partition     [IN]     Partition number (event slot)   *
   * @return Status code indicating failure or success.
   */
  virtual StatusCode clearStore( size_t partitionIndex ) = 0;

  /** Set the number of 'slots'.
   *
   * @param  partition     [IN]     Partition number (event slot)   *
   * @return Status code indicating failure or success.
   */
  virtual StatusCode setNumberOfStores( size_t slots ) = 0;

  /** Get the number of 'slots'.
   *
   * @return Number of event stores allocated in the whiteboard
   */
  virtual size_t getNumberOfStores() const = 0;

  /** Get the latest new data objects registred in store.
   *
   * @param  products     [IN]     Partition number (event slot)   *
   * @return Status code indicating failure or success.
   */
  virtual StatusCode getNewDataObjects( DataObjIDColl& products ) = 0;

  /** Check if something is new in the whiteboard without getting the products.
   *
   * @param  products     [IN]     Partition number (event slot)   *
   * @return Boolean indicating the presence of new products
   */
  virtual bool newDataObjectsPresent() = 0;

  /** Allocate a store partition for new event
   *
   * @param     evtnumber     [IN]     Event number
   * @param     partition     [OUT]    Returned slot or partition number
   * @return Partition number (npos to indicate an error).
   */
  virtual size_t allocateStore( int evtnumber ) = 0;

  /** Free a store partition
   *
   * @param     partition     [IN]     Partition number
   * @return Status code indicating failure or success.
   */
  virtual StatusCode freeStore( size_t partitionIndex ) = 0;

  /** Get the partition number corresponding to a given event
   *
   * @param     evtnumber     [IN]     Event number
   * @return    Partition number (npos to indicate an error).
   */
  virtual size_t getPartitionNumber( int eventnumber ) const = 0;
};
#endif // GAUDIKERNEL_IHIVEWHITEBOARD_H
