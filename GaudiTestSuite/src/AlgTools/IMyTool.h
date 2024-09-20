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
#ifndef GAUDIEXAMPLE_IMYTOOL_H
#define GAUDIEXAMPLE_IMYTOOL_H 1

// Include files
#include <GaudiKernel/IAlgTool.h>

/** @class IMyTool IMyTool.h
 *  Example of an Interface of a Algorithm Tool
 *
 *  @author Pere Mato
 */
class IMyTool : virtual public IAlgTool {
public:
  /// InterfaceID
  DeclareInterfaceID( IMyTool, 2, 0 );

  /// Get a message
  virtual const std::string& message() const = 0;
  /// Do something
  virtual void doIt() const = 0;
};

/** @class IMyOtherTool IMyTool.h
 *
 *  Second interface, to test multiple interface tools
 *
 *  @author Chris Jones
 */
class IMyOtherTool : virtual public IAlgTool {
public:
  /// InterfaceID
  DeclareInterfaceID( IMyOtherTool, 2, 0 );

  /// Do something else
  virtual void doItAgain() const = 0;
};

/// Interface (not implemented) to test attempts to retrieval of wrong interface
class IWrongTool : virtual public IAlgTool {
public:
  /// InterfaceID
  DeclareInterfaceID( IWrongTool, 1, 0 );
};

#endif // GAUDIEXAMPLE_IMYTOOL_H
