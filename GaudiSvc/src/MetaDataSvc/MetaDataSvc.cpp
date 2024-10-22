/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
/*
 * MetaDataSvc.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: Ana Trisovic
 */

// Framework include files
#include <Gaudi/Interfaces/IOptionsSvc.h>
#include <GaudiKernel/IAlgManager.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/IService.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/IToolSvc.h>

#include "MetaDataSvc.h"

using Gaudi::MetaDataSvc;

DECLARE_COMPONENT( MetaDataSvc )

StatusCode MetaDataSvc::start() {
  if ( msgLevel( MSG::DEBUG ) ) debug() << "started" << endmsg;
  return collectData();
}

std::map<std::string, std::string> MetaDataSvc::getMetaDataMap() const { return m_metadata; }

StatusCode MetaDataSvc::collectData() {

  // save options for all clients
  for ( const auto& p : serviceLocator()->getOptsSvc().items() ) { m_metadata[std::get<0>( p )] = std::get<1>( p ); }

  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "Metadata collected:\n";
    for ( const auto& item : m_metadata ) { debug() << item.first << ':' << item.second << '\n'; }
    debug() << endmsg;
  }

  return StatusCode::SUCCESS;
}
