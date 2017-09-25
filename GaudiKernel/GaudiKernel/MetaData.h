
/*
 * MetaData.h
 *
 *  Created on: 20 Jan 2015
 *      Author: Ana Trisovic
 */

#ifndef METADATA_H_
#define METADATA_H_
#include "DataObject.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/ISvcLocator.h"

#include <map>
#include <string>

// Definition of the CLID for this class
static const CLID CLID_MetaData = 13507;

class MetaData
{ //: public DataObject {
private:
  std::map<std::string, std::string> m_metadata;

public:
  MetaData();
  MetaData( std::map<std::string, std::string> n_m );
  virtual ~MetaData();
  std::map<std::string, std::string> getMetaDataMap();
};

#endif /* METADATA_H_ */
