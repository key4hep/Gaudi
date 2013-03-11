#ifndef GAUDIHIVE_MINIMALDATAOBJECTHANDLE_H
#define GAUDIHIVE_MINIMALDATAOBJECTHANDLE_H

#include "GaudiKernel/IDataObjectHandle.h"
#include "GaudiKernel/IAlgorithm.h"

class MinimalDataObjectHandle : public implements1<IDataObjectHandle> {

public:        
  /// Constructor initialises the members and registers products in SchedSvc
  MinimalDataObjectHandle(const std::string& productName,
                          IAlgorithm* fatherAlg,
                          AccessType accessType,
                          bool isOptional);

  /// Initialize
  StatusCode initialize();
  
  /// Finalize
  StatusCode finalize();  
  
  /// Reinitialize
  StatusCode reinitialize();
  
  /// Check if the data object declared is optional for the algorithm
  bool isOptional() const;

  /// Return the product index
  unsigned int dataProductIndex() const;

  /// Return the product name
  const std::string& dataProductName() const;
  
  /// Access type
  AccessType accessType() const ;
  
  /// Check if operation was performed
  bool wasRead() const;
  
  /// Check if operation was performed
  bool wasWritten() const;    
  
protected:
  const std::string m_dataProductName;
  IAlgorithm* m_fatherAlg;
  void setRead(bool wasRead=true);
  void setWritten(bool wasWritten=true);  
  
private:  
  const bool m_isOptional;
  const unsigned int m_dataProductIndex;
  const AccessType m_accessType;
  bool m_wasRead;
  bool m_wasWritten;    
  
  // Temporary there waiting the mapping interface to be there
  static unsigned int m_tmp_dpi;  
  
  MinimalDataObjectHandle(const MinimalDataObjectHandle& );
  MinimalDataObjectHandle& operator=(const MinimalDataObjectHandle& );
  
};


#endif