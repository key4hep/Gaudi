/***********************************************************************************\
* (c) Copyright 2013-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

/// @author Marco Clemencic <marco.clemencic@cern.ch>
/// See @ref GaudiPluginService-readme

#include <Gaudi/Details/PluginServiceCommon.h>

#if GAUDI_PLUGIN_SERVICE_USE_V2
#  include <Gaudi/PluginServiceV2.h>
#else
#  include <Gaudi/PluginServiceV1.h>
#endif
