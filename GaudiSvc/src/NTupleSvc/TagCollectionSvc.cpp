//====================================================================
//  TagCollectionSvc.cpp
//--------------------------------------------------------------------
//
//  Package   : GaudiSvc/TagCollectionSvc ( The LHCb Offline System)
//
//  Description: implementation of the NTuple service
//
//  Author    : M.Frank
//  History   :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 29/10/98| Initial version                              | MF
// | 29/09/99| Added access to ICnvManager for missing      |
// |         | converters                                   | MF
// | 20/09/00| Connect dynamically to conversion service    |
// |         | for N-tuple persistency                      | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#define GAUDISVC_TAGCOLLECTIONSVC_CPP

// Framework include files
#include "GaudiKernel/AttribStringParser.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/ObjectFactory.h"
#include "GaudiKernel/SmartIF.h"

#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ISvcManager.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Property.h"

#include "TagCollectionSvc.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT( TagCollectionSvc )

/// Standard Constructor
TagCollectionSvc::TagCollectionSvc( const std::string& name, ISvcLocator* svc ) : NTupleSvc( name, svc ) {}

/// Add file to list I/O list
StatusCode TagCollectionSvc::connect( const std::string& ident, std::string& logname )
{
  DataObject* pO    = nullptr;
  StatusCode status = findObject( m_rootName, pO );
  if ( status.isSuccess() ) {
    status = INVALID_ROOT;
    if ( pO->registry() ) {
      char typ = 0;
      std::vector<Prop> props;
      long loc = ident.find( " " );
      std::string filename, svc = "DbCnvSvc";
      logname      = ident.substr( 0, loc );
      using Parser = Gaudi::Utils::AttribStringParser;
      // we assume that there is always a " "
      // (but if it is not there, we probably will not match the pattern)
      for ( auto attrib : Parser( ident.substr( loc + 1 ) ) ) {
        switch (::toupper( attrib.tag[0] ) ) {
        case 'A':
          props.emplace_back( "Server", attrib.value );
          break;
        case 'F': /* FILE='<file name>' */
        case 'D': /* DATAFILE='<file name>' */
          filename = std::move( attrib.value );
          break;
        case 'O': /* OPT='<NEW<CREATE,WRITE>, UPDATE, READ>' */
          switch (::toupper( attrib.value[0] ) ) {
          case 'C':
          case 'N':
          case 'W':
            typ = 'N';
            break;
          case 'U':
            typ = 'U';
            break;
          case 'O':
          case 'R':
            switch (::toupper( attrib.value[2] ) ) {
            case 'C': /* RECREATE */
              typ = 'R';
              break;
            case 'A': /* READ     */
            default:
              typ = 'O';
              break;
            }
            break;
          default:
            typ = 0;
            break;
          }
          break;
        case 'S': // SVC='<service type>'
          switch (::toupper( attrib.tag[1] ) ) {
          case 'V':
            svc = std::move( attrib.value );
            break;
          case 'H':
            switch (::toupper( attrib.value[0] ) ) {
            case 'Y':
              props.emplace_back( "ShareFiles", attrib.value );
              break;
            }
            break;
          }
          break;
        case 'T': // TYP='<HBOOK,ROOT,OBJY,...>'
          switch (::toupper( attrib.value[0] ) ) {
          case 'H':
            svc = "HbookCnv::ConvSvc";
            break;
          case 'P':
            props.emplace_back( "DbType", attrib.value );
            svc = "PoolDbCnvSvc";
            break;
          default:
            props.emplace_back( "DbType", attrib.value );
            svc = "DbCnvSvc";
            break;
          }
          break;
        default:
          props.emplace_back( attrib.tag, attrib.value );
          break;
        }
      }
      if ( 0 != typ ) {
        IConversionSvc* pSvc = nullptr;
        status               = createService( name() + '.' + logname, svc, props, pSvc );
        if ( status.isSuccess() ) {
          status = attachTuple( filename, logname, typ, pSvc->repSvcType() );
          if ( status.isSuccess() ) {
            m_connections.emplace( m_rootName + '/' + logname, Connection( pSvc ) );
            return StatusCode::SUCCESS;
          }
        }
      }
    }
  }
  error() << "Cannot add " << ident << " invalid filename!" << endmsg;
  return StatusCode::FAILURE;
}

/// Create conversion service
StatusCode TagCollectionSvc::createService( const std::string& nam, const std::string& typ,
                                            const std::vector<Prop>& props, IConversionSvc*& pSvc )
{
  pSvc = nullptr;
  using Gaudi::Utils::TypeNameString;
  auto mgr = serviceLocator()->as<ISvcManager>();

  // TagCollectionSvc has to directly create a ConversionSvc to manage it directly.
  StatusCode status = NO_INTERFACE;
  if ( mgr ) {
    SmartIF<IService> isvc = mgr->createService( TypeNameString( nam, typ ) );
    if ( isvc ) {
      auto icsvc = isvc.as<IConversionSvc>();
      if ( icsvc ) {
        auto iprop = isvc.as<IProperty>();
        if ( iprop ) {
          for ( const auto& p : props ) {
            iprop->setProperty( p.first, p.second ).ignore();
          }
          // NTupleSvc has to directly create a ConversionSvc to manage it directly.
          status = isvc->sysInitialize();
          if ( status.isSuccess() ) {
            status = icsvc->setDataProvider( this );
            if ( status.isSuccess() ) {
              pSvc = icsvc.get();
              pSvc->addRef();
            }
          }
        }
      }
    }
  }
  return status;
}
