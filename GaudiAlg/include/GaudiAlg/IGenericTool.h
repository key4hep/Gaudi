/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIALG_IGENERICTOOL_H
#define GAUDIALG_IGENERICTOOL_H 1

// Include files
#include "GaudiKernel/IAlgTool.h"

/** @class IGenericTool IGenericTool.h GaudiAlg/IGenericTool.h
 *  General purpose interface class for tools that "do something"
 *
 *  @author Marco Cattaneo
 *  @date   2005-12-14
 */
class GAUDI_API IGenericTool : virtual public IAlgTool {
public:
  /// InterfaceID
  DeclareInterfaceID( IGenericTool, 2, 0 );

  /// Do the action
  virtual void execute() = 0;

  virtual ~IGenericTool() = default;
};
#endif // KERNEL_INORMALIZETOOL_H
