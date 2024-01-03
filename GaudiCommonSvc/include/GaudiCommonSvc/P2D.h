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
#pragma once

#include "Generic2D.h"
#include <AIDA/IProfile1D.h>
#include <Gaudi/Histograming/Sink/Utils.h>
#include <GaudiKernel/DataObject.h>
#include <TProfile2D.h>

#include <nlohmann/json.hpp>

#include <mutex>

namespace Gaudi {

  /**@class Profile2D
   *
   * AIDA implementation for 2 D profiles using ROOT TProfile2D
   *
   * @author  M.Frank
   */
  class GAUDI_API Profile2D : public DataObject, public Generic2D<AIDA::IProfile2D, TProfile2D> {
  public:
    /// Default Constructor
    Profile2D() : Base( new TProfile2D() ) {
      m_classType = "IProfile2D";
      m_rep->SetErrorOption( "s" );
      m_rep->SetDirectory( nullptr );
    }
    /// Default Constructor with representation object
    Profile2D( TProfile2D* rep );

    /// Fill bin content
    bool fill( double x, double y, double z, double weight ) override {
      // avoid race conditions when filling the profile
      auto guard = std::scoped_lock{ m_fillSerialization };
      m_rep->Fill( x, y, z, weight );
      return true;
    }
    friend void reset( Profile2D& h ) { h.reset(); }
    /// conversion to json via nlohmann library
    friend void to_json( nlohmann::json& j, Profile2D const& p ) { j = *p.m_rep.get(); }
    /// Retrieve reference to class defininition identifier
    const CLID&        clID() const override { return classID(); }
    static const CLID& classID() { return CLID_ProfileH2; }

  private:
    std::mutex m_fillSerialization;
  };
} // namespace Gaudi
