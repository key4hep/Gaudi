#ifndef GAUDIHIVE_MINIMALDATAOBJECTHANDLE_H
#define GAUDIHIVE_MINIMALDATAOBJECTHANDLE_H

#include "GaudiKernel/StatusCode.h"

#include <vector>
#include <string>
#include <map>

#ifndef __GCCXML__
#include <unordered_map>
#endif

#include <tbb/spin_mutex.h>

class DataObjectDescriptor;
class DataObjectDescriptorCollection;
class Algorithm;
class AlgTool;

class MinimalDataObjectHandle {

public:
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

  friend class Algorithm;
  friend class AlgTool;
  friend class DataObjectDescriptorCollection;

public:

  MinimalDataObjectHandle();

  /// Constructor initialises the members and registers products in SchedSvc
  /// the MinimalDataObjectHandle takes over the ownership of the descriptor
  MinimalDataObjectHandle(DataObjectDescriptor & descriptor);

  virtual ~MinimalDataObjectHandle();

  /// Initialize
  virtual StatusCode initialize();

  /// Finalize
  virtual StatusCode finalize();

  /// Reinitialize -> may be overwritten in derived class
  virtual StatusCode reinitialize();

  /// Check if the data object declared is optional for the algorithm
  bool isOptional() const;
  void setOptional(bool optional = true);

  /// Return the product index
  size_t dataProductIndex() const;

  /// Return the product name
  const std::string& dataProductName() const;
  const std::vector<std::string> & alternativeDataProductNames() const;


  /// Update address of data product if possible -> not if was written
  StatusCode setDataProductName(const std::string & address);
  StatusCode setAlternativeDataProductNames(const std::vector<std::string> & alternativeAddresses);
  StatusCode setDataProductNames(const std::vector<std::string> & addresses);

  /// Access type
  AccessType accessType() const ;

  /// Check if operation was performed
  bool wasRead() const;

  /// Check if operation was performed
  bool wasWritten() const;

  bool isValid() const;

  bool initialized() const { return m_initialized; }

  void lock();
  void unlock();

protected:
  DataObjectDescriptor * m_descriptor;

  void setRead(bool wasRead=true);
  void setWritten(bool wasWritten=true);

  DataObjectDescriptor * descriptor();

private:

  size_t updateDataProductIndex();

  unsigned int m_dataProductIndex;
  bool m_wasRead;
  bool m_wasWritten;
  bool m_initialized;

  //static map to hold the dataproduct index mapping
  //is shared between all events in flight
#ifndef __GCCXML__
  static std::unordered_map<std::string, size_t> m_dataProductIndexMap;
#else
  static std::map<std::string, size_t> m_dataProductIndexMap;
#endif

  //map to hold locks per event in flight
  static std::map<size_t, std::map<size_t, tbb::spin_mutex> > m_locks;
  static const uint CLEANUP_THRESHOLD = 20;

  MinimalDataObjectHandle(const MinimalDataObjectHandle& );
  MinimalDataObjectHandle& operator=(const MinimalDataObjectHandle& );

};


#endif
