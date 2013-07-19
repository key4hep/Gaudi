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
#define  GAUDISVC_TAGCOLLECTIONSVC_CPP

// Framework include files
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Tokenizer.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/ObjectFactory.h"
#include "GaudiKernel/GenericAddress.h"

#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IConversionSvc.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Property.h"

#include "TagCollectionSvc.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT(TagCollectionSvc)

/// Standard Constructor
TagCollectionSvc::TagCollectionSvc(const std::string& name, ISvcLocator* svc)
 : NTupleSvc(name, svc)
{
}

/// Standard Destructor
TagCollectionSvc::~TagCollectionSvc()   {
}

/// Initialize the service.
StatusCode TagCollectionSvc::initialize()     {
  StatusCode status = NTupleSvc::initialize();
  return status;
}

/// Finalize the service.
StatusCode TagCollectionSvc::finalize()     {
  StatusCode status = NTupleSvc::finalize();
  return status;
}

/// Add file to list I/O list
StatusCode TagCollectionSvc::connect(const std::string& ident, std::string& logname)    {
  MsgStream log ( msgSvc(), name() );
  DataObject* pO = 0;
  StatusCode status = findObject(m_rootName, pO);
  if ( status.isSuccess() )   {
    status = INVALID_ROOT;
    if ( 0 != pO->registry() )   {
      char typ=0;
      Tokenizer tok(true);
      std::vector<Prop> props;
      long loc = ident.find(" ");
      std::string filename, auth, svc = "DbCnvSvc";
      logname = ident.substr(0,loc);
      tok.analyse(ident.substr(loc+1,ident.length()), " ", "", "", "=", "'", "'");
      for ( Tokenizer::Items::iterator i = tok.items().begin(); i != tok.items().end(); i++)    {
        const std::string& tag = (*i).tag();
        switch( ::toupper(tag[0]) )   {
        case 'A':
          props.push_back( Prop("Server", (*i).value()));
          break;
        case 'F': /* FILE='<file name>' */
        case 'D': /* DATAFILE='<file name>' */
          filename = (*i).value();
          break;
        case 'O': /* OPT='<NEW<CREATE,WRITE>, UPDATE, READ>' */
          switch( ::toupper((*i).value()[0]) )   {
          case 'C':
          case 'N':
          case 'W':                   typ = 'N';            break;
          case 'U':                   typ = 'U';            break;
          case 'O':
          case 'R':
            switch( ::toupper((*i).value()[2]) )   {
            case 'C': /* RECREATE */  typ = 'R';            break;
            case 'A': /* READ     */
            default:                  typ = 'O';            break;
            }
            break;
          default:                    typ = 0;              break;
          }
          break;
        case 'S':   // SVC='<service type>'
          switch( ::toupper(tag[1]) )   {
          case 'V':                   svc = (*i).value();   break;
          case 'H':
            switch(::toupper((*i).value()[0]))  {
              case 'Y':
               props.push_back( Prop("ShareFiles", (*i).value()));
               break ;
            }
            break;
          }
          break;
        case 'T':   // TYP='<HBOOK,ROOT,OBJY,...>'
          switch(::toupper((*i).value()[0]))  {
          case 'H':
            svc = "HbookCnv::ConvSvc";
            break;
          case 'P':
            props.push_back( Prop("DbType", (*i).value()));
            svc = "PoolDbCnvSvc";
            break;
          default:
            props.push_back( Prop("DbType", (*i).value()));
            svc = "DbCnvSvc";
            break;
          }
          break;
        default:
          props.push_back( Prop((*i).tag(), (*i).value()));
          break;
        }
      }
      if ( 0 != typ )    {
        IConversionSvc* pSvc = 0;
        status = createService(name()+'.'+logname, svc, props, pSvc);
        if ( status.isSuccess() )   {
          status = attachTuple(filename,logname,typ,pSvc->repSvcType());
          if ( status.isSuccess() )    {
            m_connections.insert(Connections::value_type(m_rootName+'/'+logname,Connection(pSvc)));
            return StatusCode::SUCCESS;
          }
        }
      }
    }
  }
  log << MSG::ERROR << "Cannot add " << ident << " invalid filename!" << endmsg;
  return StatusCode::FAILURE;
}

/// Create conversion service
StatusCode TagCollectionSvc::createService( const std::string& nam,
                                            const std::string& typ,
                                            const std::vector<Prop>& props,
                                            IConversionSvc*& pSvc)    {
  using Gaudi::Utils::TypeNameString;
  SmartIF<ISvcManager> mgr(serviceLocator());

  // TagCollectionSvc has to directly create a ConversionSvc to manage it directly.
  StatusCode status = NO_INTERFACE;
  if ( mgr.isValid() )    {

    SmartIF<IService> &isvc = mgr->createService(TypeNameString(nam, typ));
    if (isvc.isValid())   {
      status = isvc->queryInterface(IConversionSvc::interfaceID(), (void**)&pSvc);
      if ( status.isSuccess() )     {
        SmartIF<IProperty> iprop(isvc);
        status = NO_INTERFACE;
        if ( iprop.isValid( ) )    {
          for ( std::vector<Prop>::const_iterator j = props.begin(); j != props.end(); j++)   {
            iprop->setProperty(StringProperty((*j).first, (*j).second)).ignore();
          }
          // NTupleSvc has to directly create a ConversionSvc to manage it directly.
          status = isvc->sysInitialize();
          if ( status.isSuccess() )   {
            status = pSvc->setDataProvider(this);
            if ( status.isSuccess() )   {
              return status;
            }
          }
        }
        pSvc->release();
      }
    }
  }
  pSvc = 0;
  return status;
}
