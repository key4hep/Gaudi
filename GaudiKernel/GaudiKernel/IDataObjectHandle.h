#ifndef GAUDIHIVE_IDATAOBJECTHANDLE_H
#define GAUDIHIVE_IDATAOBJECTHANDLE_H

// Include files
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/ClassID.h"

/** @class IDataObjectHandle IDataObjectHandle.h 
 * 
    The DataObjectHandles will be used to write, read and update data 
    objects in the event store.  

    @author Danilo Piparo
    @version 1.0
*/

class GAUDI_API IDataObjectHandle: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IDataObjectHandle,1,0);

  /// The type of the access
  // Genreflex cannot compile it with the c++11 features (strong typed + enum type)
  enum AccessType {
    /// Read only access to the data object
    READ,
    /// Data object is produced
    WRITE,
    /// Update of data object is allowed
    UPDATE
    };    
  
  /// Initialize
  virtual StatusCode initialize() = 0;

  /// Finalize
  virtual StatusCode reinitialize() = 0;  
  
  /// Finalize
  virtual StatusCode finalize() = 0;
  
  /// Check if the data object declared is optional for the algorithm
  virtual bool isOptional() const = 0;

  /// Return the product index
  virtual unsigned int dataProductIndex() const = 0;

  /// Return the product index
  virtual const std::string& dataProductName() const = 0;
  
  /// Access type
  virtual AccessType accessType() const = 0;
  
  /// Check if read operation was performed
  virtual bool wasRead() const = 0;

  /// Check if write operation was performed
  virtual bool wasWritten() const = 0;      
  
};

#endif // GAUDIHIVE_IDATAOBJECTHANDLE_H
