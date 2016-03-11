/*
 * MetaDataSvc.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: Ana Trisovic
 */

// Framework include files
#include "GaudiKernel/xtoa.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/ObjectFactory.h"
#include "GaudiKernel/GenericAddress.h"

#include "GaudiKernel/Property.h"
#include "GaudiKernel/Selector.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/MetaData.h"
#include "GaudiKernel/ConversionSvc.h"
#include "GaudiKernel/DataSelectionAgent.h"
#include "GaudiKernel/NTupleImplementation.h"

#include "GaudiKernel/Bootstrap.h"

#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SmartIF.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/GaudiException.h"

#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/DataIncident.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IConverter.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IDataSelector.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/IJobOptionsSvc.h"

#include "MetaDataSvc.h"

DECLARE_COMPONENT(MetaDataSvc)

// Standard Constructor.
//   Input:  name   String with service name
//   Input:  svc    Pointer to service locator interface

MetaDataSvc::MetaDataSvc( const std::string& name, ISvcLocator* svc )
: base_class(name, svc) {
  declareProperty("Enable", m_isEnabled = true);
}
// Initialize the service.
StatusCode MetaDataSvc::initialize() {
  StatusCode sc = Service::initialize();
  if ( sc.isFailure() ) return sc;
  return sc;
}
bool MetaDataSvc::isEnabled( ) const {
  return m_isEnabled;
}
StatusCode MetaDataSvc::start(){
  if (msgLevel(MSG::DEBUG)) debug() << "started" << endmsg;
  return collectData();
}
MetaDataSvc::~MetaDataSvc() {
}
MetaData* MetaDataSvc::getMetaData() {
  return new MetaData(m_metadata);
}
std::map <std::string, std::string> MetaDataSvc::getMetaDataMap() {
  return m_metadata;
}
StatusCode MetaDataSvc::collectData ()
{
  std::string temp;
  bool first{false};

  auto appMgr = service<IProperty>("ApplicationMgr");
  if( appMgr.isValid() ) {
    const std::vector<Property*> properties = appMgr->getProperties();
    for( int i= 0; i< (signed)properties.size(); i++ ){
      temp="ApplicationMgr." + properties[i]->name();
      m_metadata[temp]=properties[i]->toString();
    }
  }
  auto mSvc = service<IProperty>("MessageSvc");
  if( mSvc.isValid() ) {
    const std::vector<Property*> properties2 = mSvc->getProperties();
    for( int i= 0; i< (signed)properties2.size(); i++ ){
      temp="MessageSvc." + properties2[i]->name();
      m_metadata[temp]=properties2[i]->toString();
    }
  }

  auto nSvc = service<IProperty>("NTupleSvc");
  if( nSvc.isValid() ) {
    const std::vector<Property*> properties5 = nSvc->getProperties();
    for( int i= 0; i< (signed)properties5.size(); i++ ){
      temp="NTupleSvc." + properties5[i]->name();
      m_metadata[temp]=properties5[i]->toString();
    }
  }

  /*
   * TOOLS
   * */
  SmartIF<IToolSvc> tSvc(serviceLocator()->service("ToolSvc"));
  if(tSvc.isValid() ) {
    std::vector<std::string> TSvcs = tSvc->getInstances("");
    temp="";
    for( int i= 0; i< (signed)TSvcs.size(); i++ ){
      temp +=  TSvcs[i];
      if (i!=(signed)TSvcs.size()-1)temp+=", ";
    }
    m_metadata["ToolSvc"]=temp;
  }

  /*
   * SERVICES
   * */
  auto Svcs = serviceLocator()->getServices();
  temp = "";
  first = true;
  for (auto svc: Svcs) {
    if (!first)
      temp += ", ";
    else
      first = false;
    temp += svc->name();
  }
  m_metadata["ISvcLocator.Services"]=temp;

  /*
   * ALGORITHMS
   * */
  SmartIF<IAlgManager> algMan(serviceLocator());
  auto allAlgs = algMan->getAlgorithms();
  temp = "";
  first = true;
  for(auto alg: allAlgs) {
    if (!first)
      temp += ", ";
    else
      first = false;
    temp += alg->name();
  }
  m_metadata["IAlgManager.Algorithms"] = temp;

  /*
   * JOB OPTIONS SERVICE
   * */
  auto joSvc = service<IProperty>("JobOptionsSvc");
  if( !joSvc.isValid() ) { return StatusCode::FAILURE ;}
  const std::vector<Property*> properties7 = joSvc->getProperties();
  for( int i= 0; i< (signed)properties7.size(); i++ ){
    temp="JobOptionsSvc." + properties7[i]->name();
    m_metadata[temp]=properties7[i]->toString();
  }

  if (msgLevel(MSG::DEBUG)) {
    std::stringstream ss_metadata;
    for(auto item: m_metadata) {
      ss_metadata << item.first
                  << ':'
                  << item.second
                  << '\n';
    }
    debug() << "Metadata collected:\n" << ss_metadata.str() << endmsg;
  }

  return StatusCode::SUCCESS;
}
