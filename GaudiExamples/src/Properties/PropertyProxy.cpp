// Include files
#include "PropertyProxy.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"

// Static Factory declaration

DECLARE_COMPONENT( PropertyProxy )

// Constructor
//------------------------------------------------------------------------------
PropertyProxy::PropertyProxy( const std::string& name, ISvcLocator* ploc ) : Algorithm( name, ploc )
{
  //------------------------------------------------------------------------------
  // Declare remote properties at this moment
  auto algMgr = serviceLocator()->as<IAlgManager>();
  SmartIF<IProperty> rAlgP( algMgr->algorithm( "PropertyAlg" ) );
  if ( rAlgP ) {
    m_remAlg = rAlgP; // remember it for later
    declareRemoteProperty( "RInt", rAlgP, "Int" );
    declareRemoteProperty( "String", rAlgP );
  } else {
    warning() << " The 'remote' Algorithm PropertyAlg is not found" << endmsg;
  }
}

//------------------------------------------------------------------------------
StatusCode PropertyProxy::initialize()
{
  //------------------------------------------------------------------------------
  std::string value( "empty" );
  std::string value1( "empty" );

  this->getProperty( "RInt", value ).ignore();
  info() << " Got property this.RInt = " << value << ";" << endmsg;

  this->setProperty( "RInt", "1001" ).ignore();
  info() << " Set property this.RInt = "
         << "1001"
         << ";" << endmsg;

  this->getProperty( "RInt", value ).ignore();
  info() << " Got property this.RInt = " << value << ";" << endmsg;

  this->getProperty( "String", value ).ignore();
  m_remAlg->getProperty( "String", value1 ).ignore();
  if ( value == value1 ) {
    info() << " Got property this.String = " << value << ";" << endmsg;
  } else {
    error() << " Local property [" << value1 << "] not equal [" << value << "]" << endmsg;
  }

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode PropertyProxy::execute()
{
  //------------------------------------------------------------------------------
  info() << "executing...." << endmsg;

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode PropertyProxy::finalize()
{
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;

  return StatusCode::SUCCESS;
}
