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
#pragma once

#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/DataObjectHandle.h>
#include <GaudiKernel/IScheduler.h>
#include <GaudiKernel/RegistryEntry.h>
#include <GaudiKernel/RndmGenerators.h>

//------------------------------------------------------------------------------

/** @class ViewTester
 *
 *
 */
namespace Test {

  class ViewTester : public Algorithm {

  public:
    bool isClonable() const override { return true; }

    /// the execution of the algorithm
    StatusCode execute() override;
    /// Its initialization
    StatusCode initialize() override;

    ViewTester( const std::string& name, // the algorithm instance name
                ISvcLocator*       pSvc );     // the Service Locator

  private:
    /// the default constructor is disabled
    ViewTester(); // no default constructor
    /// the copy constructor is disabled
    ViewTester( const ViewTester& ); // no copy constructor
    /// the assignement operator is disabled
    ViewTester& operator=( const ViewTester& ); // no assignement

    Gaudi::Property<std::vector<std::string>>                  m_inpKeys{ this, "inpKeys", {}, "" };
    Gaudi::Property<std::vector<std::string>>                  m_outKeys{ this, "outKeys", {}, "" };
    std::vector<std::unique_ptr<DataObjectHandle<DataObject>>> m_inputHandles;
    std::vector<std::unique_ptr<DataObjectHandle<DataObject>>> m_outputHandles;

    // View config
    Gaudi::Property<std::string>  m_baseViewName{ this, "baseViewName", "view",
                                                 "Views to be named this, plus a numerical index" };
    Gaudi::Property<unsigned int> m_viewNumber{ this, "viewNumber", 0, "How many views to make" };
    Gaudi::Property<std::string>  m_viewNodeName{ this, "viewNodeName", "viewNode", "Name of node to attach views to" };
  };
} // namespace Test
