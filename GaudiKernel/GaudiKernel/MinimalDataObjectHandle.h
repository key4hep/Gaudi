#ifndef GAUDIHIVE_MINIMALDATAOBJECTHANDLE_H
#define GAUDIHIVE_MINIMALDATAOBJECTHANDLE_H

#include "GaudiKernel/IDataObjectHandle.h"
#include <vector>

class DataObjectDescriptor;

class MinimalDataObjectHandle : public implements1<IDataObjectHandle> {

public:        
  /// Constructor initialises the members and registers products in SchedSvc
  MinimalDataObjectHandle(DataObjectDescriptor & descriptor);

  /// Initialize
  StatusCode initialize();
  
  /// Finalize
  StatusCode finalize();  
  
  /// Reinitialize -> may be overwritten in derived class
  virtual StatusCode reinitialize();
  
  /// Check if the data object declared is optional for the algorithm
  bool isOptional() const;

  /// Return the product index
  unsigned int dataProductIndex() const;

  /// Return the product name
  const std::string& dataProductName() const;
  

  /// Update address of data product if possible -> not if was written
  StatusCode setDataProductName(const std::string & address);
  StatusCode setDataProductNames(const std::vector<std::string> & addresses);

  /// Access type
  AccessType accessType() const ;
  
  /// Check if operation was performed
  bool wasRead() const;
  
  /// Check if operation was performed
  bool wasWritten() const;

  bool isValid() const;
  
  bool initialized() const { return m_initialized; }

  const static std::string NULL_ADDRESS;

protected:
  DataObjectDescriptor & m_descriptor;
  void setRead(bool wasRead=true);
  void setWritten(bool wasWritten=true);  
  
private:
  const unsigned int m_dataProductIndex;
  bool m_wasRead;
  bool m_wasWritten;
  bool m_initialized;
  
  // Temporary there waiting the mapping interface to be there
  static unsigned int m_tmp_dpi;  
  
  MinimalDataObjectHandle(const MinimalDataObjectHandle& );
  MinimalDataObjectHandle& operator=(const MinimalDataObjectHandle& );
  
};


#endif
