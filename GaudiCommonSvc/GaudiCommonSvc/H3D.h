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
#ifndef GAUDICOMMONSVC_H3D_H
#define GAUDICOMMONSVC_H3D_H

#include "Generic3D.h"
#include <AIDA/IHistogram3D.h>
#include <GaudiKernel/DataObject.h>
#include <TH3D.h>

namespace Gaudi {

  /**@class Histogram3D
   *
   * AIDA implementation for 3 D histograms using ROOT THD2
   *
   * @author  M.Frank
   */
  class GAUDI_API Histogram3D : public DataObject, public Generic3D<AIDA::IHistogram3D, TH3D> {
  public:
    /// Standard Constructor
    Histogram3D();
    /// Standard Constructor
    Histogram3D( TH3D* rep );

    /// Fill bin content
    bool fill( double x, double y, double z, double weight ) override;
    /// Fast filling method for a given bin. It can be also the over/underflow bin
    virtual bool setBinContents( int i, int j, int k, int entries, double height, double error, double centreX,
                                 double centreY, double centreZ );
    /// Sets the rms of the histogram.
    virtual bool setRms( double rmsX, double rmsY, double rmsZ );
    // overwrite reset
    bool reset() override;
    /// Introspection method
    void* cast( const std::string& className ) const override;
    /// Create new histogram from any AIDA based histogram
    void copyFromAida( const AIDA::IHistogram3D& h );
    /// Retrieve reference to class defininition identifier
    const CLID&        clID() const override { return classID(); }
    static const CLID& classID() { return CLID_H3D; }

  protected:
    // cache sumwx and sumwy  when setting contents since I don't have bin mean
    double m_sumwx = 0;
    double m_sumwy = 0;
    double m_sumwz = 0;

  private:
    std::mutex m_fillSerialization;
  };
} // namespace Gaudi

#endif
