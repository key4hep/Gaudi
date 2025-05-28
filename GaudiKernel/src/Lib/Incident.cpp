/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#define _inc_types_impl_
#include <GaudiKernel/Incident.h>
#include <GaudiKernel/ThreadLocalContext.h>

Incident::Incident( const std::string& source, const std::string& type )
    : m_source( source ), m_type( type ), m_ctx( Gaudi::Hive::currentContext() ) {}
