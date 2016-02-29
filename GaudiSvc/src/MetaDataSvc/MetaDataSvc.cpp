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
#include "TFile.h"
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
// Finalize the service.
StatusCode MetaDataSvc::finalize() {
  return Service::finalize();
}
bool MetaDataSvc::isEnabled( ) const {
  return m_isEnabled;
}
StatusCode MetaDataSvc::sysInitialize(){
  return Service::sysInitialize();
}
StatusCode MetaDataSvc::start(){
  return collectData();
}
StatusCode MetaDataSvc::sysFinalize(){
  return Service::sysFinalize();
}
MetaDataSvc::~MetaDataSvc() {
}
MetaData* MetaDataSvc::getMetaData() {
	md = new MetaData(m);
	return md;
}
std::map <std::string, std::string> MetaDataSvc::getMetaDataMap() {
	return m;
}
StatusCode MetaDataSvc::writeMetaData(TFile* rfile) {
	if(rfile->WriteObject(&m, "info"))
	{
		return StatusCode::SUCCESS;
	}
	else return StatusCode::FAILURE;
}
StatusCode MetaDataSvc::collectData ()
{
	std::string temp;
	MsgStream log ( msgSvc() , "MetaData");

	SmartIF<IProperty> appMgr(serviceLocator()->service("ApplicationMgr"));
	if( appMgr.isValid() ) {
	const std::vector<Property*> properties = appMgr->getProperties();
	for( int i= 0; i< (signed)properties.size(); i++ ){
		    temp="ApplicationMgr." + properties[i]->name();
			m[temp]=properties[i]->toString();
		  }
	}
	SmartIF<IProperty> mSvc(serviceLocator()->service("MessageSvc"));
	if( mSvc.isValid() ) {
	const std::vector<Property*> properties2 = mSvc->getProperties();
	for( int i= 0; i< (signed)properties2.size(); i++ ){
		        temp="MessageSvc." + properties2[i]->name();
				m[temp]=properties2[i]->toString();
			  }
	}

	SmartIF<IProperty> nSvc(serviceLocator()->service("NTupleSvc"));
	if( nSvc.isValid() ) {
	const std::vector<Property*> properties5 = nSvc->getProperties();
	for( int i= 0; i< (signed)properties5.size(); i++ ){
				temp="NTupleSvc." + properties5[i]->name();
				m[temp]=properties5[i]->toString();
			  }
	}

	/*
	 * TOOLS
	 * */
	SmartIF<IToolSvc> tSvc(serviceLocator()->service("ToolSvc"));
	if(tSvc.isValid() ) {
		std::vector<std::string> TSvcs = tSvc->getInstances(""); // tool type in brackets ??
		temp="";
		for( int i= 0; i< (signed)TSvcs.size(); i++ ){
						temp +=  TSvcs[i];
						if (i!=(signed)TSvcs.size()-1)temp+=", ";
					  }
		m["ToolSvc"]=temp;
	}


	/*
	 * SERVICES
	 * */
	std::list<IService*> Svcs =serviceLocator()->getServices( );
	std::list<IService*>::const_iterator iterator;
	temp="";
	for (iterator = Svcs.begin(); iterator != Svcs.end(); ++iterator) {
		temp+= (*iterator) -> name();
		if (Svcs.back() != (*iterator))temp+=", ";
	}
	m["ISvcLocator.Services"]=temp;

	/*
	 * ALGORITHMS
	 * */
	SmartIF<IAlgManager> algMan(serviceLocator());
	std::list<IAlgorithm*> allAlgs = algMan->getAlgorithms();
	std::list<IAlgorithm*>::const_iterator ialg;
	temp="";
	for(ialg = allAlgs.begin() ; allAlgs.end() != ialg ; ++ialg ) {
		temp+= (*ialg) -> name();
		if (allAlgs.back()!= (*ialg))temp+=", ";
	}
	m["IAlgManager.Algorithm"]=temp;

	/*
	 * JOB OPTIONS SERVICE
	 * */
	SmartIF<IProperty> joSvc(serviceLocator()->service("JobOptionsSvc"));
	if( !joSvc.isValid() ) { return StatusCode::FAILURE ;}
	const std::vector<Property*> properties7 = joSvc->getProperties();
	for( int i= 0; i< (signed)properties7.size(); i++ ){
				temp="JobOptionsSvc." + properties7[i]->name();
				m[temp]=properties7[i]->toString();
	}

	log << MSG::INFO << "Job information collected" << endmsg;
	return StatusCode::SUCCESS;
}
