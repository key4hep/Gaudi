#ifndef GAUDIKERNEL_DATAHANDLECONFIGURABLE_H
#define GAUDIKERNEL_DATAHANDLECONFIGURABLE_H 1

#include <functional>
#include <string>

#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/IDataHandleMetadata.h"

namespace Gaudi
{
  /// Configurable entity associated with a DataHandle property
  ///
  /// This class combines access to the IDataHandleMetadata with a
  /// configurable target DataObjID. It is what the Property machinery will
  /// eventually interact with.
  ///
  class DataHandleConfigurable
  {
  public:
    /// Initialize by providing access to the metadata singleton
    DataHandleConfigurable( const IDataHandleMetadata& metadata, DataObjID defaultKey )
        : m_metadata( metadata ), m_key( std::move( defaultKey ) )
    {
    }

    /// Propagate metadata of the underlying DataHandle type
    const IDataHandleMetadata& metadata() const { return m_metadata; }

    /// Tell what the currently configured target is
    const DataObjID& targetKey() const { return m_key; }

    /// Change the target of the data handle
    void setTargetKey( const DataObjID& key ) { m_key = key; }

    /// Boilerplate for Gaudi::Property compatibility
    friend std::ostream& operator<<( std::ostream& str, const DataHandleConfigurable& d );

  private:
    std::reference_wrapper<const IDataHandleMetadata> m_metadata;
    DataObjID                                         m_key;
  };

  // More boilerplate for Gaudi::Property compatibility
  namespace Parsers
  {
    GAUDI_API StatusCode parse( Gaudi::DataHandleConfigurable&, const std::string& );
  }
  namespace Utils
  {
    GAUDI_API std::ostream& toStream( const Gaudi::DataHandleConfigurable&, std::ostream& );
  }
}

#endif
