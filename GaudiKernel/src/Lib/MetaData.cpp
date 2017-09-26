/*
 * MetaData.cpp
 *
 *  Created on: 20 Jan 2015
 *      Author: Ana Trisovic
 */

#include "GaudiKernel/MetaData.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SmartIF.h"

#include "GaudiKernel/MsgStream.h"

#include "GaudiKernel/IJobOptionsSvc.h"

#include "GaudiKernel/PropertyHolder.h"
#include "GaudiKernel/Service.h"

#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IConverter.h"
#include "GaudiKernel/IOpaqueAddress.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/xtoa.h"

#include "GaudiKernel/DataIncident.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/IAlgTool.h"

#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IToolSvc.h"

MetaData::MetaData() {}
MetaData::MetaData( std::map<std::string, std::string> n_m )
{
  m_metadata.clear();
  m_metadata.insert( n_m.begin(), n_m.end() );
}
MetaData::~MetaData() {}
std::map<std::string, std::string> MetaData::getMetaDataMap() { return m_metadata; }
