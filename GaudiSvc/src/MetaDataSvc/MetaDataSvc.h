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

namespace Gaudi {
class MetaDataSvc : public extends<Service, IMetaDataSvc> {
public:
	using extends::extends;
	~MetaDataSvc() override = default;

	StatusCode start() override;

	inline bool isEnabled() const { return m_isEnabled; }

	StatusCode collectData();

	MetaData* getMetaData() override;
	std::map <std::string, std::string> getMetaDataMap() override;

private:
	BooleanProperty m_isEnabled{this, "Enabled", true};

	std::map <std::string, std::string> m_metadata;
};
}

#endif /* GAUDISVC_SRC_METADATASVC_METADATASVC_H_ */
