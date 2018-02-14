#define GAUDIKERNEL_CONVERSIONSVC_CPP

#include "GaudiKernel/ConversionSvc.h"
#include "GaudiKernel/Converter.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IConverter.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/System.h"

namespace
{

  auto CnvTest = []( CLID clid ) { return [clid]( const auto& i ) { return i.clID() == clid; }; };

  enum CnvSvcAction {
    CREATE_OBJ,
    FILL_OBJ_REFS,
    UPDATE_OBJ,
    UPDATE_OBJ_REFS,
    CREATE_REP,
    FILL_REP_REFS,
    UPDATE_REP,
    UPDATE_REP_REFS
  };
}

StatusCode ConversionSvc::makeCall( int typ, bool ignore_add, bool ignore_obj, bool update, IOpaqueAddress*& pAddress,
                                    DataObject*& pObject )
{
  if ( !pAddress && !ignore_add ) return Status::INVALID_ADDRESS;
  if ( !pObject && !ignore_obj ) return Status::INVALID_OBJECT;
  const CLID& obj_class =
      ( pObject && !ignore_obj ) ? pObject->clID() : ( pAddress && !ignore_add ) ? pAddress->clID() : CLID_NULL;
  IConverter* cnv = converter( obj_class );
  if ( !cnv && pObject ) {
    // Give it a try to autoload the class (dictionary) for which the converter is needed
    loadConverter( pObject );
    cnv = converter( obj_class );
  }

  StatusCode status( StatusCode::FAILURE, true );
  if ( cnv ) {
    switch ( typ ) {
    case CREATE_OBJ:
      pObject = nullptr;
      status  = cnv->createObj( pAddress, pObject );
      break;
    case FILL_OBJ_REFS:
      status = cnv->fillObjRefs( pAddress, pObject );
      break;
    case UPDATE_OBJ:
      status = cnv->updateObj( pAddress, pObject );
      break;
    case UPDATE_OBJ_REFS:
      status = cnv->updateObjRefs( pAddress, pObject );
      break;
    case CREATE_REP:
      pAddress = nullptr;
      status   = cnv->createRep( pObject, pAddress );
      break;
    case FILL_REP_REFS:
      status = cnv->fillRepRefs( pAddress, pObject );
      break;
    case UPDATE_REP:
      status = cnv->updateRep( pAddress, pObject );
      break;
    case UPDATE_REP_REFS:
      status = cnv->updateRepRefs( pAddress, pObject );
      break;
    default:
      status = StatusCode::FAILURE;
      break;
    }
    if ( status.isSuccess() && update ) {
      status = updateServiceState( pAddress );
    }
    return status;
  }
  status.ignore();
  info() << "No converter for object ";
  if ( pObject ) {
    msgStream() << System::typeinfoName( typeid( *pObject ) );
  }
  msgStream() << "  CLID= " << obj_class << endmsg;
  return Status::NO_CONVERTER;
}

void ConversionSvc::loadConverter( DataObject* ) {}

StatusCode ConversionSvc::updateServiceState( IOpaqueAddress* /*pAddress */ ) { return StatusCode::SUCCESS; }

/// Create the transient representation of an object.
StatusCode ConversionSvc::createObj( IOpaqueAddress* pAddress, DataObject*& refpObj )
{
  return makeCall( CREATE_OBJ, false, true, false, pAddress, refpObj );
}

/// Resolve the references of the created transient object.
StatusCode ConversionSvc::fillObjRefs( IOpaqueAddress* pAddress, DataObject* pObj )
{
  return makeCall( FILL_OBJ_REFS, false, true, true, pAddress, pObj );
}

/// Update the transient object from the other representation.
StatusCode ConversionSvc::updateObj( IOpaqueAddress* pAddress, DataObject* pObj )
{
  return makeCall( UPDATE_OBJ, false, true, false, pAddress, pObj );
}

/// Update the references of an updated transient object.
StatusCode ConversionSvc::updateObjRefs( IOpaqueAddress* pAddress, DataObject* pObj )
{
  return makeCall( UPDATE_OBJ_REFS, false, true, true, pAddress, pObj );
}

/// Convert the transient object to the requested representation.
StatusCode ConversionSvc::createRep( DataObject* pObj, IOpaqueAddress*& refpAddress )
{
  return makeCall( CREATE_REP, true, false, false, refpAddress, pObj );
}

/// Resolve the references of the converted object.
StatusCode ConversionSvc::fillRepRefs( IOpaqueAddress* pAddress, DataObject* pObj )
{
  return makeCall( FILL_REP_REFS, true, false, false, pAddress, pObj );
}

/// Update the converted representation of a transient object.
StatusCode ConversionSvc::updateRep( IOpaqueAddress* pAddress, DataObject* pObj )
{
  return makeCall( UPDATE_REP, true, false, false, pAddress, pObj );
}

/// Update the references of an already converted object.
StatusCode ConversionSvc::updateRepRefs( IOpaqueAddress* pAddress, DataObject* pObj )
{
  return makeCall( UPDATE_REP_REFS, true, false, false, pAddress, pObj );
}

/// Retrieve converter from list
IConverter* ConversionSvc::converter( const CLID& clid )
{
  IConverter* cnv                 = nullptr;
  auto i                          = std::find_if( m_workers.begin(), m_workers.end(), CnvTest( clid ) );
  if ( i != m_workers.end() ) cnv = i->converter();
  if ( !cnv ) {
    StatusCode status = addConverter( clid );
    if ( status.isSuccess() ) {
      i                               = std::find_if( m_workers.begin(), m_workers.end(), CnvTest( clid ) );
      if ( i != m_workers.end() ) cnv = i->converter();
    }
  }
  return cnv;
}

/// Define transient data store
StatusCode ConversionSvc::setDataProvider( IDataProviderSvc* pDataSvc )
{
  if ( !pDataSvc ) return StatusCode::SUCCESS; // Atlas does not use DataSvc
  m_dataSvc = pDataSvc;
  for ( auto& i : m_workers ) {
    IConverter* cnv = i.converter();
    if ( cnv && cnv->setDataProvider( m_dataSvc ).isFailure() ) {
      error() << "setting Data Provider" << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}

/// Access the transient store
SmartIF<IDataProviderSvc>& ConversionSvc::dataProvider() const { return m_dataSvc; }

/// Set address creator facility
StatusCode ConversionSvc::setAddressCreator( IAddressCreator* creator )
{
  m_addressCreator = creator;
  for ( auto& i : m_workers ) {
    auto* cnv = i.converter();
    if ( cnv ) {
      if ( cnv->setAddressCreator( m_addressCreator ).isFailure() ) {
        error() << "setting Address Creator" << endmsg;
      }
    }
  }
  return StatusCode::SUCCESS;
}

/// Access the transient store
SmartIF<IAddressCreator>& ConversionSvc::addressCreator() const { return m_addressCreator; }

/// Set conversion service the converter is connected to
StatusCode ConversionSvc::setConversionSvc( IConversionSvc* /* svc */ ) { return StatusCode::FAILURE; }

/// Get conversion service the converter is connected to
SmartIF<IConversionSvc>& ConversionSvc::conversionSvc() const { return m_cnvSvc; }

/// Add converter object to conversion service.
StatusCode ConversionSvc::addConverter( const CLID& clid )
{
  // First look for the more specific converter
  long typ               = repSvcType();
  IConverter* pConverter = createConverter( typ, clid, nullptr );
  if ( pConverter ) {
    StatusCode status = configureConverter( typ, clid, pConverter );
    if ( status.isSuccess() ) {
      status = initializeConverter( typ, clid, pConverter );
      if ( status.isSuccess() ) {
        status = activateConverter( typ, clid, pConverter );
        if ( status.isSuccess() ) {
          long conv_typ         = pConverter->repSvcType();
          const CLID& conv_clid = pConverter->objType();
          typ                   = ( typ < 0xFF ) ? typ : typ & 0xFFFFFF00;
          conv_typ              = ( conv_typ < 0xFF ) ? conv_typ : conv_typ & 0xFFFFFF00;
          if ( conv_typ == typ && conv_clid == clid ) {
            return addConverter( pConverter );
          }
        }
      }
    }
    pConverter->release();
  }
  return Status::NO_CONVERTER;
}

/// Add converter object to conversion service.
StatusCode ConversionSvc::addConverter( IConverter* pConverter )
{
  if ( pConverter ) {
    const CLID& clid = pConverter->objType();
    removeConverter( clid ).ignore();
    m_workers.emplace_back( clid, pConverter );
    return StatusCode::SUCCESS;
  }
  return Status::NO_CONVERTER;
}

/// Remove converter object from conversion service (if present).
StatusCode ConversionSvc::removeConverter( const CLID& clid )
{

  auto i = std::partition( std::begin( m_workers ), std::end( m_workers ),
                           [f = CnvTest( clid )]( const WorkerEntry& we ) { return !f( we ); } );
  if ( i == std::end( m_workers ) ) return Status::NO_CONVERTER;
  std::for_each( i, std::end( m_workers ), []( WorkerEntry& w ) { w.converter()->finalize().ignore(); } );
  m_workers.erase( i, std::end( m_workers ) );
  return StatusCode::SUCCESS;
}

/// Initialize the service.
StatusCode ConversionSvc::initialize() { return Service::initialize(); }

/// stop the service.
StatusCode ConversionSvc::finalize()
{
  // Release all workers.
  for ( auto& i : m_workers ) {
    if ( i.converter()->finalize().isFailure() ) {
      error() << "finalizing worker" << endmsg;
    }
  }
  m_workers.clear();
  // release interfaces
  m_addressCreator = nullptr;
  m_dataSvc        = nullptr;
  m_cnvSvc         = nullptr;
  return Service::finalize();
}

/// Create new Converter using factory
IConverter* ConversionSvc::createConverter( long typ, const CLID& clid, const ICnvFactory* /*fac*/ )
{
  IConverter* pConverter = Converter::Factory::create( ConverterID( typ, clid ), serviceLocator().get() );
  if ( !pConverter ) {
    typ        = ( typ < 0xFF ) ? typ : typ & 0xFFFFFF00;
    pConverter = Converter::Factory::create( ConverterID( typ, clid ), serviceLocator().get() );
  }
  return pConverter;
}

/// Configure the freshly created converter
StatusCode ConversionSvc::configureConverter( long /* typ */, const CLID& /* clid */, IConverter* pConverter )
{
  if ( !pConverter ) return Status::NO_CONVERTER;
  pConverter->setConversionSvc( this ).ignore();
  pConverter->setAddressCreator( m_addressCreator ).ignore();
  pConverter->setDataProvider( m_dataSvc ).ignore();
  return StatusCode::SUCCESS;
}

/// Initialize new converter
StatusCode ConversionSvc::initializeConverter( long /* typ */, const CLID& /* clid */, IConverter* pConverter )
{
  return pConverter ? pConverter->initialize() : Status::NO_CONVERTER;
}

/// Activate the freshly created converter
StatusCode ConversionSvc::activateConverter( long /* typ */, const CLID& /* clid */, IConverter* pConverter )
{
  return pConverter ? StatusCode::SUCCESS : StatusCode( Status::NO_CONVERTER );
}

/// Retrieve the class type of objects the converter produces.
const CLID& ConversionSvc::objType() const { return CLID_NULL; }

/// Retrieve the class type of the data store the converter uses.
long ConversionSvc::repSvcType() const { return m_type; }

/// Connect the output file to the service with open mode.
StatusCode ConversionSvc::connectOutput( const std::string& outputFile, const std::string& /* openMode */ )
{
  return connectOutput( outputFile );
}

/// Connect the output file to the service.
StatusCode ConversionSvc::connectOutput( const std::string& /* outputFile */ ) { return StatusCode::SUCCESS; }

/// Commit pending output.
StatusCode ConversionSvc::commitOutput( const std::string&, bool ) { return StatusCode::SUCCESS; }

/// Create a Generic address using explicit arguments to identify a single object.
StatusCode ConversionSvc::createAddress( long /* svc_type */, const CLID& /* clid     */,
                                         const std::string* /* par      */, const unsigned long* /* ip       */,
                                         IOpaqueAddress*& refpAddress )
{
  refpAddress = nullptr;
  return StatusCode::FAILURE;
}

/// Convert an address to string form
StatusCode ConversionSvc::convertAddress( const IOpaqueAddress* /* pAddress */, std::string& refAddress )
{
  refAddress.clear();
  return StatusCode::FAILURE;
}

/// Convert an address in string form to object form
StatusCode ConversionSvc::createAddress( long /* svc_type */, const CLID& /* clid */,
                                         const std::string& /* refAddress */, IOpaqueAddress*& refpAddress )
{
  refpAddress = nullptr;
  return StatusCode::FAILURE;
}

/// Standard Constructor
ConversionSvc::ConversionSvc( const std::string& name, ISvcLocator* svc, long type )
    : base_class( name, svc ), m_cnvSvc( this )
{
  m_type    = type;
  m_dataSvc = nullptr;
  setAddressCreator( this ).ignore();
}
