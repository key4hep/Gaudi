// $Id: AIDATupleSvc.cpp,v 1.5 2006/11/15 10:45:18 hmd Exp $
// Include files
#include <cstdlib>

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/Tokenizer.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/IRegistry.h"

#include "AIDATupleSvc.h"

//-------------------------------------
//For myTest(). REMOVE:
#include "CLHEP/Random/RandGauss.h"
#include "CLHEP/Random/DRand48Engine.h"
#include <math.h>
//-------------------------------------
// Handle CLHEP 2.0.x move to CLHEP namespace
namespace CLHEP { }
using namespace CLHEP;

/**  AIDATupleSvc class implementation
 *
 *  @author Manuel Domenech Saavedra
 *  @date   2005-05-23
 */

StatusCode AIDATupleSvc::myTest()
{
  MsgStream log ( msgSvc(), name() );
  log << MSG::INFO << "Welcome to myTest!" << endmsg;

  // Defining the description of the tuple columns
  std::string description =  "float px; float py; float pz; float mass";

  pi_aida::Proxy_Store* store = new pi_aida::Proxy_Store("ntuple.root","POOL",0,"CAT=xmlcatalog_file:anotherCatalog.xml,SUB=POOL_ROOTTREE");
  pi_aida::Tuple tuple(*store, "1", "tuple1", description);
  //AIDATuple tuple( store, "1", "example tuple", description );

  DRand48Engine randomEngine;
  RandGauss rBeamEnergy( randomEngine, 90, 5 );
  RandGauss rTracksSpread( randomEngine, 0, 2 );
  RandGauss rMomentum( randomEngine, 0, 3 );
  RandGauss rMass( randomEngine, 1, 0.1 );

  std::cout << "Tuple created ... starting to fill " << std::endl;

  int i_px = tuple.findColumn( "px" );
  int i_py = tuple.findColumn( "py" );
  int i_pz = tuple.findColumn( "pz" );
  int i_mass = tuple.findColumn( "mass" );

  for ( unsigned int i = 0; i < 1000; i++ ) {

      tuple.fill( i_px,  rMomentum.fire() );
      tuple.fill( i_py,  rMomentum.fire() );
      tuple.fill( i_pz,  rMomentum.fire() );
      tuple.fill( i_mass, rMass.fire() );
      tuple.addRow();
  }
  std::cout << "Filled the tuple with " << tuple.rows() << " rows" << std::endl;
  store->close();
  delete store;

  pi_aida::Proxy_Store* store1 = new pi_aida::Proxy_Store("ntuple.root","POOL",3,"CAT=xmlcatalog_file:anotherCatalog.xml,SUB=POOL_ROOTTREE");
  pi_aida::Tuple tuple1 = store1->retrieveTuple("1");

  //AIDATuple tuple1 = store1->retrieveTuple("1");
  //AIDATuple tuple = static_cast<AIDATuple>(tmp);

  std::cout << "FOUND TUPLE WITH  " << tuple1.rows() << " ROWS" << std::endl;

  tuple1.start();
  int i = 0;
  while( tuple1.next() ) {
    i++;
    std::cout << "ROW " << i << std::endl;
    std::cout << "px: " << tuple1.getFloat(i_px) << std::endl;
    std::cout << "py: " << tuple1.getFloat(i_py) << std::endl;
    std::cout << "pz: " << tuple1.getFloat(i_pz) << std::endl;
    std::cout << "mass: " << tuple1.getFloat(i_mass) << std::endl;
  }
  store1->close();
  delete store1;
  return StatusCode::SUCCESS;
}


// ==============================================
// Methods to book a tuple in the transient store
// ==============================================
AIDA::ITuple*	AIDATupleSvc::book(	const std::string& fullPath,
                                  const std::string& title,
                                  const std::string& columns	)
{
  std::string dirPath, objPath, storePath, storeObj;
  parsePath( fullPath, dirPath, objPath, storePath, storeObj);

  Connections::iterator i = m_connections.find(storePath);
  if( i != m_connections.end() ) {
    pi_aida::Tuple* tmp = new pi_aida::Tuple( *(*i).second, storeObj, title, columns );
    AIDA::ITuple* tuple = tmp;

    if( registerObject(dirPath, objPath, tuple).isSuccess() )
      return tuple;
    else {
      delete tuple;
      throw GaudiException("Cannot book tuple " + title,"AIDATupleSvc", StatusCode::FAILURE);
    }
  }
  else
    throw GaudiException("Cannot find store " + storePath, "AIDATupleSvc", StatusCode::FAILURE);
}


AIDA::ITuple*	AIDATupleSvc::book(	const std::string& parentPath,
                                  const std::string& objPath,
                                  const std::string& title,
                                  const std::string& columns	)
{
  std::string fullPath = parentPath+"/"+objPath;
  return book( fullPath, title, columns );
}


AIDA::ITuple*	AIDATupleSvc::book(	const std::string& parentPath,
                                  int tupleID,
                                  const std::string& title,
                                  const std::string& columns	)
{
  char objPath[32];
  ::_itoa(tupleID, objPath, 10);
  std::string fullPath = parentPath+"/"+objPath;
  return book( fullPath, title, columns );
}


AIDA::ITuple*	AIDATupleSvc::book(	DataObject* pParent,
                                  const std::string& objPath,
                                  const std::string& title,
                                  const std::string& columns	)
{
  IRegistry* tmp = pParent->registry();
  std::string fullPath, parentPath;
  parentPath = tmp->identifier();
  fullPath = fullPath = parentPath+"/"+objPath;
  return book( fullPath, title, columns );
}


AIDA::ITuple* AIDATupleSvc::book(	DataObject* pParent,
                                  int tupleID,
                                  const std::string& title,
                                  const std::string& columns	)
{
  char txt[32];
  return book( pParent, ::_itoa(tupleID, txt, 10), title, columns );
}


// ==================================================
// Methods to register a tuple in the transient store
// ==================================================
StatusCode AIDATupleSvc::registerObject( const std::string& dirPath,
                                         const std::string& objPath,
                                         AIDA::ITuple* tObj )
{
  DataObject* pParent = 0;
  pParent = createPath( dirPath );
  return registerObject( pParent, objPath, tObj );

}

StatusCode AIDATupleSvc::registerObject( DataObject* pParent,
                                         const std::string& objPath,
                                         AIDA::ITuple* tObj )
{
  // Set the tuple id
  if ( objPath[0] == SEPARATOR ) {
    if ( !tObj->annotation().addItem( "id", objPath.substr(1) ) )
      tObj->annotation().setValue( "id", objPath.substr(1) );
  }
  else {
    if ( !tObj->annotation().addItem( "id", objPath ) )
      tObj->annotation().setValue( "id", objPath );
  }

  // Register the tuple in the tuple data store
  StatusCode status = DataSvc::registerObject( pParent,
                                               objPath,
                                               dynamic_cast<DataObject*>(tObj) );
  return status;
}


// ====================================================
// Methods to unregister a tuple in the transient store
// ====================================================
StatusCode AIDATupleSvc::unregisterObject( AIDA::ITuple* tObj )
{
  StatusCode status = DataSvc::unregisterObject( dynamic_cast<DataObject*>(tObj) );
  return status;
}


StatusCode AIDATupleSvc::unregisterObject( AIDA::ITuple* tObj,
                                           const std::string& objPath )
{
  StatusCode status = DataSvc::unregisterObject( dynamic_cast<DataObject*>(tObj),
                                                 objPath );
  return status;
}


// ===============================================
// Methods to retrieve a tuple from the data store
// ===============================================
StatusCode AIDATupleSvc::retrieveObject( const std::string& fullPath,
                                         AIDA::ITuple*& tObj )
{
  MsgStream log ( msgSvc(), name() );
  StatusCode status;
  status = findObject(fullPath, tObj);

  // Tuple already in the transient store
  if( status.isSuccess() )
    return status;

  std::string dirPath, objPath, storePath, storeObj;
  parsePath( fullPath, dirPath, objPath, storePath, storeObj);

  // Load tuple from persistency
  Connections::iterator i = m_connections.find(storePath);
  if( i != m_connections.end() ) {
    pi_aida::Tuple* tmp = new pi_aida::Tuple(((*i).second)->retrieveTuple(storeObj));

    tObj = dynamic_cast<AIDA::ITuple*>(tmp);
    return StatusCode::SUCCESS;
  }
  log << MSG::ERROR << "Could not retrieve tuple " << fullPath << endmsg;
  return StatusCode::FAILURE;
}


StatusCode AIDATupleSvc::retrieveObject( const std::string& parentPath,
                                         const std::string& objPath,
                                         AIDA::ITuple*& tObj )
{
  std::string fullPath = parentPath+'/'+objPath;
  return retrieveObject( fullPath, tObj );
}


StatusCode AIDATupleSvc::retrieveObject( DataObject* pParent,
                                         const std::string& objPath,
                                         AIDA::ITuple*& tObj )
{
  IRegistry* tmpReg = pParent->registry();
  std::string parentPath = tmpReg->identifier();
  std::string fullPath = parentPath+'/'+objPath;

  return retrieveObject( fullPath, tObj );
}


// =========================================
// Methods to find a tuple in the data store
// =========================================
StatusCode AIDATupleSvc::findObject( const std::string& fullPath,
                                     AIDA::ITuple*& tObj )
{
  DataObject*  pObject  = 0;
  StatusCode sc = DataSvc::findObject( fullPath, pObject );
  tObj = dynamic_cast<AIDA::ITuple*>(pObject);
  return sc;
}


StatusCode AIDATupleSvc::findObject( IRegistry* pRegistry,
                                     const std::string& path,
                                     AIDA::ITuple*& tObj )
{
  DataObject*  pObject  = 0;
  StatusCode sc = DataSvc::findObject( pRegistry, path, pObject );
  tObj = dynamic_cast<AIDA::ITuple*>(pObject);
  return sc;
}


StatusCode AIDATupleSvc::findObject( const std::string& parentPath,
                                     const std::string& objPath,
                                     AIDA::ITuple*& tObj )
{
  DataObject*  pObject  = 0;
  StatusCode sc = DataSvc::findObject( parentPath, objPath, pObject );
  tObj = dynamic_cast<AIDA::ITuple*>(pObject);
  return sc;
}


StatusCode AIDATupleSvc::findObject( DataObject* parentObj,
                                     const std::string& objPath,
                                     AIDA::ITuple*& tObj )
{
  DataObject*  pObject  = 0;
  StatusCode sc = DataSvc::findObject( parentObj, objPath, pObject );
  tObj = dynamic_cast<AIDA::ITuple*>(pObject);
  return sc;
}


void AIDATupleSvc::setCriteria( AIDA::ITuple*& tObj,
				const std::string& criteria )
{
  pi_aida::Tuple* t = dynamic_cast<pi_aida::Tuple*>(tObj);
  t->setCriteria(criteria);
}


// ==============
// Helper methods
// ==============

// std::string AIDATupleSvc::storeName(AIDA::ITuple*& tObj)
// {
//   pi_aida::Tuple* t = dynamic_cast<pi_aida::Tuple*>(tObj);
//   std::cout << "NAME: " << t->storeName() << std::endl;
//   return t->storeName();
// }


StatusCode AIDATupleSvc::connect( const std::string& ident,
                                  int mode )
{
  MsgStream log ( msgSvc(), name() );
  DataObject* p0 = 0;
  StatusCode status = DataSvc::findObject(m_rootName, p0);
  if (status.isSuccess() ) {
    Tokenizer tok(true);
    long loc = ident.find(" ");
    int open_mode = 0; // Default is AUTO mode
    std::string file, typ = "", tmp_typ = "", opt = "";
    std::string logname = ident.substr(0,loc);
    tok.analyse(ident.substr(loc+1,ident.length()), " ", "", "", "=", "'", "'");
    for ( Tokenizer::Items::iterator i = tok.items().begin(); i != tok.items().end(); i++) {
      const std::string& tag = (*i).tag();
      switch( ::toupper(tag[0]) ) {
      case 'F':   // FILE='<file name>'
      case 'D':   // DATAFILE='<file name>'
        file = (*i).value();
        break;
      case 'O':   // OPT='<AUTO, OVERWRITE, CREATE, READONLY, UPDATE>'
        switch( ::toupper((*i).value()[0]) ) {
        case 'A': // AUTO
          if( mode == 0 )
            open_mode = 3; // READONLY
          else
            open_mode = 1; // CREATE
          break;
        case 'O': // OVERWRITE
          if( mode == 0 )
            throw GaudiException("Input file " + file + " with improper option (OVERWRITE)", "AIDATupleSvc", StatusCode::FAILURE);
          else
            open_mode = 1;
          break;
        case 'C': // CREATE
          if( mode == 0 )
            throw GaudiException("Input file " + file + " with improper option (CREATE)", "AIDATupleSvc", StatusCode::FAILURE);
          else
            open_mode = 2;
          break;
        case 'R': // READONLY
          if( mode == 1 )
            throw GaudiException("Output file " + file + " with improper option (READONLY)", "AIDATupleSvc", StatusCode::FAILURE);
          else
            open_mode = 3;
          break;
        case 'U': // UPDATE
          if( mode == 0 )
            throw GaudiException("Input file " + file + " with improper option (UPDATE)", "AIDATupleSvc", StatusCode::FAILURE);
          else
            open_mode = 4;
          break;
        }
        break;
      case 'T':   // TYP='<HBOOK,ROOT,XML,POOL>'
        typ = (*i).value();
        break;
      default:
        opt += (*i).tag() + "=" + (*i).value() + ",";
        break;
      }
    }

    // Check if persistency type has been specified at job options
    if (typ == "") {
      log << MSG::WARNING << "File type not specified at job options."
          << " Setting 'HistogramPersistency' type as default"
          << endmsg;
      // Get the value of the Stat persistancy mechanism from the AppMgr
      IProperty*   appPropMgr = 0;
      status = serviceLocator()->queryInterface(IProperty::interfaceID(),(void **)&appPropMgr );
      if( !status.isSuccess() ) {
        // Report an error and return the FAILURE status code
        log << MSG::ERROR << "Could not get PropMgr" << endmsg;
        return status;
      }
      StringProperty sp("HistogramPersistency","");
      status = appPropMgr->getProperty( &sp );
      if ( !status.isSuccess() ) {
        log << MSG::ERROR << "Could not get NTuple Persistency format"
            << " from ApplicationMgr properties" << endmsg;
        return status;
      }
      tmp_typ = sp.value();
      if (tmp_typ == "ROOT" || tmp_typ == "XML" || tmp_typ == "HBOOK" || tmp_typ == "POOL")
        typ = tmp_typ;
      else {
        log << MSG::ERROR << "Unknown persistency format at 'HistogramPersistency' option"
            << endmsg;
        return StatusCode::FAILURE;
      }
    }

    // Create a new pi_aida::Proxy_Store
    pi_aida::Proxy_Store* storePtr = new pi_aida::Proxy_Store( file, typ, open_mode, opt );
    m_connections.insert(Connections::value_type(m_rootName+'/'+logname, storePtr));

    return StatusCode::SUCCESS;
  }
  log << MSG::ERROR << "Cannot add " << ident << " invalid filename!" << endmsg;
  return StatusCode::FAILURE;

}


StatusCode AIDATupleSvc::closeAIDAStores()
{
   for( Connections::iterator k = m_connections.begin(); k != m_connections.end(); k++ ) {
     if ( (*k).second->isOpen()) {
       (*k).second->close();
       (*k).second = 0;
     }
   }

   m_connections.erase(m_connections.begin(), m_connections.end());
  return StatusCode::SUCCESS;
}


void AIDATupleSvc::parsePath( const std::string& fullPath,
                              std::string& dirPath,
                              std::string& objPath,
                              std::string& storePath,
                              std::string& storeObj )
{
  std::string tmpPath = fullPath;
  if ( tmpPath[0] != SEPARATOR ) {
    // Insert the top level name of the store (/NTUPLES)
    tmpPath.insert(tmpPath.begin(), SEPARATOR);
    tmpPath.insert(tmpPath.begin(), m_rootName.begin(), m_rootName.end());
  }
  // Remove trailing "/" from tmpPath if it exists
  if (tmpPath.rfind(SEPARATOR) == tmpPath.length()-1) {
    tmpPath.erase(tmpPath.rfind(SEPARATOR),1);
  }
  int sep   = tmpPath.rfind(SEPARATOR);
  dirPath   = tmpPath.substr(0, sep);
  objPath   = tmpPath.substr(sep, tmpPath.length()-sep );
  sep       = tmpPath.find(SEPARATOR, m_rootName.length()+1);
  storePath = tmpPath.substr(0, sep);
  storeObj  = tmpPath.substr(sep+1, tmpPath.length()-sep );
}


DataObject* AIDATupleSvc::createPath( const std::string& newPath )
{
  std::string tmpPath = newPath;
  if ( tmpPath[0] != SEPARATOR )    {
    tmpPath.insert(tmpPath.begin(), SEPARATOR);
    tmpPath.insert(tmpPath.begin(), m_rootName.begin(), m_rootName.end());
  }
  // Remove trailing "/" from newPath if it exists
  if (tmpPath.rfind(SEPARATOR) == tmpPath.length()-1) {
    tmpPath.erase(tmpPath.rfind(SEPARATOR),1);
  }

  DataObject* pObject = 0;
  StatusCode sc = DataSvc::findObject( tmpPath, pObject );
  if( sc.isSuccess() ) {
    return pObject;
  }

  int sep = tmpPath.rfind(SEPARATOR);
  std::string rest( tmpPath, sep+1, tmpPath.length()-sep );
  std::string subPath( tmpPath, 0, sep );
  if( 0 != sep ) {
    createPath( subPath );
  }
  else {
    MsgStream log( msgSvc(), name() );
    log << MSG::ERROR << "Unable to create the tuple path" << endmsg;
    return 0;
  }

  pObject = createDirectory( subPath, rest );
  return pObject;
}


DataObject* AIDATupleSvc::createDirectory( const std::string& parentDir,
                                           const std::string& subDir )
{

  StatusCode   status    = StatusCode::FAILURE;
  DataObject*  directory = new DataObject();

  if ( 0 != directory )  {
    DataObject* pnode;
    status = DataSvc::findObject( parentDir, pnode );
    if( status.isSuccess() ) {
      status = DataSvc::registerObject( pnode, subDir, directory );
      if ( !status.isSuccess() )   {
        MsgStream log( msgSvc(), name() );
        log << MSG::ERROR << "Unable to create the directory: "
                          << parentDir << "/" << subDir << endmsg;
        delete directory;
        return 0;
      }
    }
    else {
      MsgStream log( msgSvc(), name() );
      log << MSG::ERROR << "Unable to create the directory: "
                        << parentDir << "/" << subDir << endmsg;
      delete directory;
      return 0;
    }
  }
  return directory;
}

// ========================================
// Constructor, destructor and main methods
// ========================================
AIDATupleSvc::AIDATupleSvc( const std::string& name, ISvcLocator* svc )
  : DataSvc( name, svc )
{
  // Properties can be declared here
  m_rootName = "/NTUPLES";
  m_rootCLID = CLID_DataObject;
  declareProperty("Input",  m_input);
  declareProperty("Output", m_output);
}


AIDATupleSvc::~AIDATupleSvc()
{
  clearStore().ignore();
}


StatusCode AIDATupleSvc::initialize()
{
  MsgStream log( msgSvc(), name() );
  StatusCode status = DataSvc::initialize();

  if( status.isSuccess() ){
    DataObject* rootObj = new DataObject();
    status = setRoot( m_rootName, rootObj );
    if( !status.isSuccess() ) {
      log << MSG::ERROR << "Unable to set NTuple data store root." << endmsg;
      delete rootObj;
      return status;
    }

    //Connect inputs (MODE = 0)
    for( DBaseEntries::iterator i = m_input.begin(); i != m_input.end(); i++ ) {
      status = connect(*i, 0);
      if( !status.isSuccess() )
        return status;
    }
    //Connect outputs (MODE = 1)
    for( DBaseEntries::iterator j = m_output.begin(); j != m_output.end(); j++ ) {
      status = connect(*j, 1);
      if( !status.isSuccess() )
        return status;
    }
  }
  return status;
}


StatusCode AIDATupleSvc::reinitialize()
{
  return StatusCode::SUCCESS;
}


StatusCode AIDATupleSvc::finalize()
{
  MsgStream log( msgSvc(), name() );
  StatusCode status = DataSvc::finalize();
  if( status.isSuccess() ) {
    status = closeAIDAStores();
    if( status.isSuccess() )
      return status;
  }
  log << MSG::ERROR << "Error finalizing AIDATupleSvc." << endmsg;
  return status;
}

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_SERVICE_FACTORY(AIDATupleSvc)
