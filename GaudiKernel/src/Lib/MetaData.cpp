/*
 * MetaData.cpp
 *
 *  Created on: 20 Jan 2015
 *      Author: Ana Trisovic
 */

#include "GaudiKernel/MetaData.h"

MetaData::MetaData( std::map<std::string, std::string> data ) : m_metadata( std::move( data ) ) {}
