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
// Framework include files
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/MsgStream.h"

#include "../AlgTools/IMyTool.h"
#include "GaudiKernel/AlgTool.h"

// Accessing data:
#include "GaudiKernel/PhysicalConstants.h"

namespace bug_34121 { // see https://its.cern.ch/jira/browse/GAUDI-200

  // Tool example
  class Tool : public extends<AlgTool, IMyTool> {
  public:
    /// Standard Constructor
    using extends::extends;

    /// IMyTool interface
    const std::string& message() const override;
    void               doIt() const override;
    /// Overriding initialize and finalize
    StatusCode initialize() override;
    StatusCode finalize() override;

  private:
    /// Properties
    Gaudi::Property<double> m_double{this, "Double", 100.};
  };

  //------------------------------------------------------------------------------
  const std::string& Tool::message() const
  //------------------------------------------------------------------------------
  {
    static const std::string msg( "It works!!!" );
    return msg;
  }

  //------------------------------------------------------------------------------
  void Tool::doIt() const
  //------------------------------------------------------------------------------
  {
    info() << "Double = " << m_double.value() << endmsg;
  }

  //------------------------------------------------------------------------------
  StatusCode Tool::initialize()
  //------------------------------------------------------------------------------
  {
    info() << "Initialize" << endmsg;
    info() << "Double = " << m_double.value() << endmsg;
    return StatusCode::SUCCESS;
  }
  //------------------------------------------------------------------------------
  StatusCode Tool::finalize()
  //------------------------------------------------------------------------------
  {
    info() << "Finalize" << endmsg;
    return StatusCode::SUCCESS;
  }

  // Declaration of the AlgTool Factory
  DECLARE_COMPONENT( Tool )
} // namespace bug_34121
