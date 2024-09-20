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
#ifndef GAUDIKERNEL_IDODALGMAPPER_H
#define GAUDIKERNEL_IDODALGMAPPER_H 1
// Include files
// from STL
#include <string>

// from Gaudi
#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/TypeNameString.h>

/** Interface of tools used by the DataOnDemandSvc to choose the algorithm to be
 * run to produce the data at a path.
 *
 * @author Marco Clemencic
 * @date 13/01/2012
 */
class IDODAlgMapper : virtual public IAlgTool {
public:
  /// InterfaceID
  DeclareInterfaceID( IDODAlgMapper, 1, 0 );

  /// For the given path, returns a TypeNameString object identifying the algorithm
  /// to be run to produce the entry at the path.
  virtual Gaudi::Utils::TypeNameString algorithmForPath( const std::string& path ) = 0;
};

#endif // GAUDIKERNEL_IDODALGMAPPER_H
