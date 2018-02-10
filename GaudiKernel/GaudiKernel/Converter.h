#ifndef GAUDIKERNEL_CONVERTER_H
#define GAUDIKERNEL_CONVERTER_H

// generic experiment headers
#include "GaudiKernel/ConversionSvc.h"
#include "GaudiKernel/IConverter.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"

// Forward declarations
class IMessageSvc;
class IRegistry;

/** @class Converter Converter.h GaudiKernel/Converter.h

    Converter base class. See interface for detailed description,
    arguments and return values

    @author Markus Frank
    @version 1.0
*/
class GAUDI_API Converter : public implements<IConverter>
{
public:
#ifndef __REFLEX__
  typedef Gaudi::PluginService::Factory<IConverter*, ISvcLocator*> Factory;
#endif

  /// Initialize the converter
  StatusCode initialize() override;

  /// Initialize the converter
  StatusCode finalize() override;

  /// Set Data provider service
  StatusCode setDataProvider( IDataProviderSvc* svc ) override;

  /// Get Data provider service
  SmartIF<IDataProviderSvc>& dataProvider() const override;

  /// Set conversion service the converter is connected to
  StatusCode setConversionSvc( IConversionSvc* svc ) override;

  /// Get conversion service the converter is connected to
  SmartIF<IConversionSvc>& conversionSvc() const override;

  /// Set address creator facility
  StatusCode setAddressCreator( IAddressCreator* creator ) override;

  /// Retrieve address creator facility
  SmartIF<IAddressCreator>& addressCreator() const override;

  /// Retrieve the class type of objects the converter produces.
  const CLID& objType() const override;

  /// Retrieve the class type of the data store the converter uses.
  // MSF: Masked to generate compiler error due to interface change
  virtual long i_repSvcType() const;

  /// Create the transient representation of an object.
  StatusCode createObj( IOpaqueAddress* pAddress, DataObject*& refpObject ) override;

  /// Resolve the references of the created transient object.
  StatusCode fillObjRefs( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  /// Update the transient object from the other representation.
  StatusCode updateObj( IOpaqueAddress* pAddress, DataObject* refpObject ) override;

  /// Update the references of an updated transient object.
  StatusCode updateObjRefs( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  /// Convert the transient object to the requested representation.
  StatusCode createRep( DataObject* pObject, IOpaqueAddress*& refpAddress ) override;

  /// Resolve the references of the converted object.
  StatusCode fillRepRefs( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  /// Update the converted representation of a transient object.
  StatusCode updateRep( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  /// Update the references of an already converted object.
  StatusCode updateRepRefs( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  /// Standard Constructor
  Converter( long storage_type, const CLID& class_type, ISvcLocator* svc = 0 );

  /// Access a service by name, creating it if it doesn't already exist.
  template <class T>
  StatusCode service( const std::string& name, T*& psvc, bool createIf = false ) const
  {
    return service_i( name, createIf, T::interfaceID(), reinterpret_cast<void**>( &psvc ) );
  }

  /// Access a service by name, type creating it if it doesn't already exist.
  template <class T>
  StatusCode service( const std::string& type, const std::string& name, T*& psvc ) const
  {
    return service_i( type, name, T::interfaceID(), reinterpret_cast<void**>( &psvc ) );
  }

  /// Return a pointer to the service identified by name (or "type/name")
  SmartIF<IService> service( const std::string& name, const bool createIf = true ) const;

protected:
  /// Retrieve pointer to service locator
  SmartIF<ISvcLocator>& serviceLocator() const;
  /// Retrieve pointer to message service
  SmartIF<IMessageSvc>& msgSvc() const;
  /// Get Data Manager service
  SmartIF<IDataManagerSvc>& dataManager() const;

private:
  /// Storage type
  long m_storageType;
  /// Class type the converter can handle
  const CLID m_classType;
  /// Pointer to the address creation service interface
  mutable SmartIF<IAddressCreator> m_addressCreator;
  /// Pointer to data provider service
  mutable SmartIF<IDataProviderSvc> m_dataProvider;
  /// Pointer to data manager service
  mutable SmartIF<IDataManagerSvc> m_dataManager;
  /// Pointer to the connected conversion service
  mutable SmartIF<IConversionSvc> m_conversionSvc;
  /// Service Locator reference
  mutable SmartIF<ISvcLocator> m_svcLocator;
  /// MessageSvc reference
  mutable SmartIF<IMessageSvc> m_messageSvc;

  /** implementation of service method */
  StatusCode service_i( const std::string& svcName, bool createIf, const InterfaceID& iid, void** ppSvc ) const;
  StatusCode service_i( const std::string& svcType, const std::string& svcName, const InterfaceID& iid,
                        void** ppSvc ) const;
};

// Identified class for converters' factories
class GAUDI_API ConverterID final
{
public:
  ConverterID( long stype, CLID clid ) : m_stype( stype ), m_clid( clid ) {}
  inline bool operator==( const ConverterID& id ) const { return m_stype == id.m_stype && m_clid == id.m_clid; }

private:
  friend std::ostream& operator<<( std::ostream&, const ConverterID& );
  long m_stype;
  CLID m_clid;
};

inline std::ostream& operator<<( std::ostream& s, const ConverterID& id )
{
  return s << "CNV_" << id.m_stype << "_" << id.m_clid;
}

#ifndef GAUDI_NEW_PLUGIN_SERVICE
template <class T>
class CnvFactory final
{
public:
#ifndef __REFLEX__
  template <typename S, typename... Args>
  static typename S::ReturnType create( Args&&... a1 )
  {
    return new T( std::forward<Args>( a1 )... );
  }
#endif
};

// Macro to declare component factories
#define DECLARE_CONVERTER_FACTORY( x )                                                                                 \
  DECLARE_FACTORY_WITH_CREATOR_AND_ID( x, CnvFactory<x>, ConverterID( x::storageType(), x::classID() ),                \
                                       Converter::Factory )
#define DECLARE_NAMESPACE_CONVERTER_FACTORY( n, x ) DECLARE_CONVERTER_FACTORY( n::x )

#else

// Macro to declare component factories
#define DECLARE_CONVERTER_FACTORY( x ) DECLARE_COMPONENT_WITH_ID( x, ConverterID( x::storageType(), x::classID() ) )
#define DECLARE_NAMESPACE_CONVERTER_FACTORY( n, x ) DECLARE_CONVERTER_FACTORY( n::x )

#endif

#endif // GAUDIKERNEL_CONVERTER_H
