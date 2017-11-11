
/*
 * MetaData.h
 *
 *  Created on: 20 Jan 2015
 *      Author: Ana Trisovic
 */

#ifndef METADATA_H_
#define METADATA_H_

#include "GaudiKernel/ClassID.h"
#include <map>
#include <string>

// Definition of the CLID for this class
static const CLID CLID_MetaData = 13507;

class MetaData
{
private:
  std::map<std::string, std::string> m_metadata;

public:
  MetaData() = default;
  MetaData( std::map<std::string, std::string> data );
  virtual ~MetaData() = default;
  std::map<std::string, std::string> getMetaDataMap() { return m_metadata; }
};

#endif /* METADATA_H_ */
