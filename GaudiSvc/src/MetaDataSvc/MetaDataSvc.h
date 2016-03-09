/*
 * MetaDataSvc.h
 *
 *  Created on: Mar 24, 2015
 *      Author: Ana Trisovic
 */

#ifndef GAUDISVC_SRC_METADATASVC_METADATASVC_H_
#define GAUDISVC_SRC_METADATASVC_METADATASVC_H_
#include "GaudiKernel/IMetaDataSvc.h"
#include "GaudiKernel/Service.h"

class MetaDataSvc : public extends<Service, IMetaDataSvc> {
public:
	// Standard Constructor.
	//   Input:  name   String with service name
	//   Input:  svc    Pointer to service locator interface
	MetaDataSvc( const std::string& name, ISvcLocator* svc );

	// Destructor.
	~MetaDataSvc();

	StatusCode initialize();
	StatusCode start();
	bool isEnabled() const ;

	StatusCode collectData();

	MetaData* getMetaData();
	std::map <std::string, std::string> getMetaDataMap();

private:
	bool m_isEnabled;
	MetaData* md;
	std::map <std::string, std::string> m_metadata;
};

#endif /* GAUDISVC_SRC_METADATASVC_METADATASVC_H_ */
