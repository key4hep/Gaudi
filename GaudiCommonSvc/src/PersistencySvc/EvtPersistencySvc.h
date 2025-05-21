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
//	============================================================
//
//	PersistencySvc.h
//	------------------------------------------------------------
//
//	Package   : PersistencySvc
//
//	Author    : Markus Frank
//
//	===========================================================
#pragma once

#include "PersistencySvc.h"

/** EvtPersistencySvc class implementation definition.

    <P> System:  The LHCb Offline System
    <P> Package: EvtPersistencySvc

     Dependencies:
    <UL>
    <LI> PersistencySvc definition:  "Kernel/Interfaces/PersistencySvc.h"
    </UL>

    History:
    <PRE>
    +---------+----------------------------------------------+---------+
    |    Date |                 Comment                      | Who     |
    +---------+----------------------------------------------+---------+
    | 3/11/98 | Initial version                              | M.Frank |
    +---------+----------------------------------------------+---------+
    </PRE>
   @author Markus Frank
   @version 1.0
*/
class EvtPersistencySvc : virtual public PersistencySvc {
public:
  /**@name: Object implementation  */
  //@{
  /// Standard Constructor
  EvtPersistencySvc( const std::string& name, ISvcLocator* svc );
};
