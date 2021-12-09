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
#ifndef GAUDIEXANMPLES_MYGAUDITOOL_H
#define GAUDIEXANMPLES_MYGAUDITOOL_H 1

// Include files
#include "GaudiAlg/GaudiTool.h"
#include "IMyTool.h"

/** @class MyTool MyTool.h
 *  This is an interface class for a example tool
 *
 *  @author Pere Mato
 *  @date   14/10/2001
 */
class MyGaudiTool : public extends<GaudiTool, IMyTool, IMyOtherTool> {
public:
  /// Standard Constructor
  using extends::extends;

  // IMyTool interface
  const std::string& message() const override;
  void               doIt() const override;

  // IMyOtherTool interface
  void doItAgain() override;

  /// Overriding initialize and finalize
  StatusCode initialize() override;
  StatusCode finalize() override;

  /// Standard destructor
  ~MyGaudiTool() override;

private:
  /// Properties
  Gaudi::Property<int>         m_int{ this, "Int", 100 };
  Gaudi::Property<double>      m_double{ this, "Double", 100. };
  Gaudi::Property<std::string> m_string{ this, "String", "hundred" };
  Gaudi::Property<bool>        m_bool{ this, "Bool", true };
};
#endif // GAUDIEXANMPLES_MYTOOL_H
