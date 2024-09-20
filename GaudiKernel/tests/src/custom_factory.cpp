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
 * Simple example of a custom factory to test:
 *
 * GAUDI-975: relax the checks in genconf to allow custom component factories
 * https://its.cern.ch/jira/browse/GAUDI-975
 *
 */

#include <Gaudi/PluginService.h>

struct IMyInterface {
  using Factory = Gaudi::PluginService::Factory<IMyInterface*()>;
};

struct MyImplementation : virtual public IMyInterface {};

DECLARE_COMPONENT( MyImplementation )
