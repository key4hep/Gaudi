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

#include "Generic1D.h"
#include <AIDA/IProfile1D.h>
#include <Gaudi/Histograming/Sink/Utils.h>
#include <GaudiKernel/DataObject.h>
#include <TProfile.h>

#include <nlohmann/json.hpp>

/*
 *    Gaudi namespace
 */
namespace Gaudi {

  /**@class Profile1D
   *
   * AIDA implementation for 1 D profiles using ROOT TProfile
   *
   * @author  M.Frank
   */
  class GAUDI_API Profile1D : public DataObject, public Generic1D<AIDA::IProfile1D, TProfile> {
    typedef AIDA::IAnnotation IAnnotation;

  private:
    void init( const std::string& title, bool initialize_axis = true );

  public:
    /// Default Constructor
    Profile1D();
    /// Standard constructor with initialization. The histogram representation will be adopted
    Profile1D( TProfile* rep );

    /// Fill the Profile1D with a value and the corresponding weight.
    bool         fill( double x, double y, double weight = 1. ) override;
    virtual bool setBinContents( int i, int entries, double height, double error, double spread, double centre );
    friend void  reset( Profile1D& h ) { h.reset(); }
    /// conversion to json via nlohmann library
    friend void to_json( nlohmann::json& j, Profile1D const& p ) { j = *p.m_rep.get(); }
    /// Retrieve reference to class defininition identifier
    const CLID&        clID() const override { return classID(); }
    static const CLID& classID() { return CLID_ProfileH; }

  private:
    std::mutex m_fillSerialization;

  }; // end class IProfile1D
} // end namespace Gaudi
