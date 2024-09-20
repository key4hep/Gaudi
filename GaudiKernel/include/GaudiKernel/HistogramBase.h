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
#ifndef HISTOGRAMSVC_BASE_H
#define HISTOGRAMSVC_BASE_H

#include <GaudiKernel/Kernel.h>
#include <iostream>

class TObject;

/*
 *    Gaudi namespace declaration
 */
namespace Gaudi {

  /**@class HistogramBase HistogramBase.h HistogramSvc/HistogramBase.h
   *
   *   Common base class for all histograms
   *   Use is solely functional to minimize dynamic_casts inside HistogramSvc
   *
   *   @author  M.Frank
   *   @version 1.0
   */
  class GAUDI_API HistogramBase {
  public:
    /// ROOT object implementation
    virtual TObject* representation() const = 0;
    /// Adopt ROOT histogram representation
    virtual void adoptRepresentation( TObject* rep ) = 0;
    /// Print histogram to output stream
    virtual std::ostream& print( std::ostream& s ) const = 0;
    /// Write (binary) histogram to output stream
    virtual std::ostream& write( std::ostream& s ) const = 0;
    /// Write (binary) histogram to file
    virtual int write( const char* file_name ) const = 0;
    virtual ~HistogramBase()                         = default;
  }; // class

} // namespace Gaudi

#endif // HISTOGRAMSVC_BASE_H
