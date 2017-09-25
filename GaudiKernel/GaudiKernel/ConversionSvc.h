#ifndef GAUDIKERNEL_CONVERSIONSVC_H
#define GAUDIKERNEL_CONVERSIONSVC_H 1

// Include files
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/Service.h"
#include <algorithm>
#include <utility>
#include <vector>

/** @class ConversionSvc ConversionSvc.h GaudiKernel/ConversionSvc.h

    Base class for all conversion services. It manages a set of Converters that
    are specialized for conversions of specific objects.

    This supports self learning converters:
    User hooks include the following (overridable) entries:
    - findCnvFactory:      returns a suitable converter factory
    - createConverter:     the actual convetrer creation
    - configureConverter:  configure converter before initialize
    - initializeConverter: initializes the converter
    - activateConverter:   any additional configuration to be done after
                           initialize.

    configureConverter and activateConverter are user hooks, where
    the convetrer can be manipulated by the hosting service
    and knowledge can be supplied, which a "generic" converter cannot
    aquire itself.

    These hooks allow any sub-classed conversion service to
    override the calls and create converters, which aquire
    the knowledge about their persistency type and the
    object type they convert during the initialization.

    Only AFTER these three steps the converter must satisfy the
    storage type of the hosting service and the class type of
    the required object type.

    @author Markus Frank
    @version 1.0
*/
class GAUDI_API ConversionSvc : public extends<Service, IConversionSvc, IAddressCreator>
{
  class WorkerEntry final
  {
    CLID m_class;
    IConverter* m_converter;

  public:
    WorkerEntry( const CLID& cl, IConverter* cnv ) : m_class( cl ), m_converter( cnv )
    {
      if ( m_converter ) m_converter->addRef();
    }

    ~WorkerEntry()
    {
      if ( m_converter ) m_converter->release();
    }

    WorkerEntry( WorkerEntry&& orig ) noexcept
        : m_class{orig.m_class}, m_converter{std::exchange( orig.m_converter, nullptr )}
    {
    }

    WorkerEntry& operator=( WorkerEntry&& orig ) noexcept
    {
      m_class = orig.m_class;
      std::swap( m_converter, orig.m_converter );
      return *this;
    }

    WorkerEntry( const WorkerEntry& copy ) = delete;
    WorkerEntry& operator=( const WorkerEntry& copy ) = delete;

    IConverter* converter() { return m_converter; }

    const CLID& clID() const { return m_class; }
  };

public:
  /// Standard Constructor
  ConversionSvc( const std::string& name, ISvcLocator* svc, long type );

  /// disable copy and assignment
  ConversionSvc( const ConversionSvc& ) = delete;
  ConversionSvc& operator=( const ConversionSvc& ) = delete;

  /// Initialize the service.
  StatusCode initialize() override;

  /// stop the service.
  StatusCode finalize() override;

  /// Retrieve the class type of the data store the converter uses.
  long repSvcType() const override;

  /// Implementation of IConverter: dummy call
  const CLID& objType() const override;

  /** Implementation of IConverter: Set Data provider service
      @return    Status code indicating success or failure
      @param     pService   Pointer to data provider service
  */
  StatusCode setDataProvider( IDataProviderSvc* pService ) override;

  /** Implementation of IConverter: Get Data provider service
      @return    Pointer to data provider service
  */
  SmartIF<IDataProviderSvc>& dataProvider() const override;

  /// Implementation of IConverter: Set conversion service the converter is connected to
  StatusCode setConversionSvc( IConversionSvc* svc ) override;

  /// Implementation of IConverter: Get conversion service the converter is connected to
  SmartIF<IConversionSvc>& conversionSvc() const override;

  /// Set address creator facility
  StatusCode setAddressCreator( IAddressCreator* creator ) override;

  /// Retrieve address creator facility
  SmartIF<IAddressCreator>& addressCreator() const override;

  /// Implementation of IConverter: Create the transient representation of an object.
  StatusCode createObj( IOpaqueAddress* pAddress, DataObject*& refpObject ) override;

  /// Implementation of IConverter: Resolve the references of the created transient object.
  StatusCode fillObjRefs( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  /// Implementation of IConverter: Update the transient object from the other representation.
  StatusCode updateObj( IOpaqueAddress* pAddress, DataObject* refpObject ) override;

  /// Implementation of IConverter: Update the references of an updated transient object.
  StatusCode updateObjRefs( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  /// Implementation of IConverter: Convert the transient object to the requested representation.
  StatusCode createRep( DataObject* pObject, IOpaqueAddress*& refpAddress ) override;

  /// Implementation of IConverter: Resolve the references of the converted object.
  StatusCode fillRepRefs( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  /// Implementation of IConverter: Update the converted representation of a transient object.
  StatusCode updateRep( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  /// Implementation of IConverter: Update the references of an already converted object.
  StatusCode updateRepRefs( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  /// Add converter object to conversion service.
  StatusCode addConverter( const CLID& clid ) override;

  /// Add converter object to conversion service.
  StatusCode addConverter( IConverter* pConverter ) override;

  /// Remove converter object from conversion service (if present).
  StatusCode removeConverter( const CLID& clid ) override;

  /// Retrieve converter from list
  IConverter* converter( const CLID& wanted ) override;

  /// Connect the output file to the service with open mode.
  StatusCode connectOutput( const std::string& outputFile, const std::string& openMode ) override;

  /// Connect the output file to the service.
  StatusCode connectOutput( const std::string& output ) override;

  /// Commit pending output.
  StatusCode commitOutput( const std::string& output, bool do_commit ) override;

  /// Create a Generic address using explicit arguments to identify a single object.
  StatusCode createAddress( long svc_type, const CLID& clid, const std::string* par, const unsigned long* ip,
                            IOpaqueAddress*& refpAddress ) override;

  /// Convert an address to string form
  StatusCode convertAddress( const IOpaqueAddress* pAddress, std::string& refAddress ) override;

  /// Convert an address in string form to object form
  StatusCode createAddress( long svc_type, const CLID& clid, const std::string& refAddress,
                            IOpaqueAddress*& refpAddress ) override;

  /// Update state of the service
  virtual StatusCode updateServiceState( IOpaqueAddress* pAddress );

protected:
  /// Create new Converter using factory
  virtual IConverter* createConverter( long typ, const CLID& clid, const ICnvFactory* fac );

  /// Configure the new converter before initialize is called
  virtual StatusCode configureConverter( long typ, const CLID& clid, IConverter* cnv );

  /// Initialize the new converter
  virtual StatusCode initializeConverter( long typ, const CLID& clid, IConverter* cnv );

  /// Activate the new converter after initialization
  virtual StatusCode activateConverter( long typ, const CLID& clid, IConverter* cnv );

  /// Load converter or dictionary needed by the converter
  virtual void loadConverter( DataObject* pObject );

  /// Retrieve address creation interface
  virtual SmartIF<IAddressCreator>& addressCreator() { return m_addressCreator; }

protected:
  StatusCode makeCall( int typ, bool ignore_add, bool ignore_obj, bool update, IOpaqueAddress*& pAddress,
                       DataObject*& pObject );

  /// Pointer to data provider service
  mutable SmartIF<IDataProviderSvc> m_dataSvc;
  /// Pointer to the address creation service interface
  mutable SmartIF<IAddressCreator> m_addressCreator;
  /// Pointer to the IConversionSvc interface of this
  mutable SmartIF<IConversionSvc> m_cnvSvc;
  /// Conversion service type
  long m_type;
  /// List of conversion workers
  std::vector<WorkerEntry> m_workers;
};
#endif // GAUDIKERNEL_CONVERSIONSVC_H
