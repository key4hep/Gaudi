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
#ifndef ITESTTOOL_H
#define ITESTTOOL_H 1

// Include files
// from STL
#include <string>

// from Gaudi
#include <GaudiKernel/IAlgTool.h>

/** @class ITestTool ITestTool.h
 *
 *
 *  @author Chris Jones
 *  @date   2004-03-08
 */

class ITestTool : public virtual IAlgTool {

public:
  /// InterfaceID
  DeclareInterfaceID( ITestTool, 2, 0 );
  virtual ~ITestTool() {}

protected:
private:
};
#endif // ITESTTOOL_H
